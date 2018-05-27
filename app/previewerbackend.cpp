#include <previewerbackend.h>
#include <projectbackend.h>
#include <previewresult.h>
#include <previewercommands.h>
#include <delayer.h>
#include <saveutils.h>
#include <control.h>
#include <hashfactory.h>
#include <filemanager.h>

#include <QPointer>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QProcess>
#include <QApplication>

namespace {

quint32 blockSize = 0;
QString serverName;
QPointer<QLocalSocket> socket;

void send(QLocalSocket* socket, const QByteArray& data)
{
    QByteArray msg;
    QDataStream out(&msg, QIODevice::WriteOnly);
    out << quint32(data.size());
    out << data;
    socket->write(msg);
    socket->flush();
}

void terminate(QLocalSocket* socket)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << REQUEST_TERMINATE;
    send(socket, data);
    socket->abort();
    ::socket = nullptr;
}

void restart(QLocalSocket* socket, const QStringList& arguments)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << REQUEST_RESTART;
    out << arguments;
    send(socket, data);
    socket->abort();
    ::socket = nullptr;
}
}

PreviewerBackend* PreviewerBackend::s_instance = nullptr;
bool PreviewerBackend::s_disabled = false;
bool PreviewerBackend::s_dirtHandlingEnabled = false;
QLocalServer* PreviewerBackend::s_server = nullptr;
QTimer* PreviewerBackend::s_dirtHandlingDisablerTimer = nullptr;
QList<PreviewerBackend::Task> PreviewerBackend::s_taskList;

PreviewerBackend::PreviewerBackend(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_server = new QLocalServer(this);
    s_dirtHandlingDisablerTimer = new QTimer(this);

    s_server->setSocketOptions(QLocalServer::UserAccessOption);
    connect(s_server, &QLocalServer::newConnection, &PreviewerBackend::onNewConnection);
    serverName = HashFactory::generate();
    s_dirtHandlingDisablerTimer->setInterval(500);
    connect(s_dirtHandlingDisablerTimer, &QTimer::timeout, &PreviewerBackend::disableDirtHandling);
}

PreviewerBackend::~PreviewerBackend()
{
    if (socket)
        terminate(socket);
    s_instance = nullptr;
}

PreviewerBackend* PreviewerBackend::instance()
{
    return s_instance;
}

bool PreviewerBackend::init()
{
#if defined(PREVIEWER_DEBUG)
    QLocalServer::removeServer("serverName");
    return s_server->listen("serverName");
#else
    QProcess process;
    process.setProgram(qApp->applicationDirPath() + "/objectwheel-previewer");
    process.setArguments(QStringList() << serverName);
    // process.setStandardOutputFile(QProcess::nullDevice());
    // process.setStandardErrorFile(QProcess::nullDevice());
    return s_server->listen(serverName) && process.startDetached();
#endif
}

bool PreviewerBackend::isBusy()
{
    return !s_taskList.isEmpty();
}

int PreviewerBackend::totalTask()
{
    return s_taskList.size();
}

void PreviewerBackend::setDisabled(bool value)
{
    s_disabled = value;
}

bool PreviewerBackend::contains(const QString& uid)
{
    for (const auto& task : s_taskList)
        if (task.uid == uid)
            return true;
    return false;
}

void PreviewerBackend::restart()
{
    if (socket && s_taskList.isEmpty()) {
        blockSize = 0;
#if !defined(PREVIEWER_DEBUG)
        ::restart(
                    socket.data(),
                    QStringList()
                    << ProjectBackend::dir()
                    << serverName
                    );
#endif
    } else {
        static bool retryAvailable = true;
        if (retryAvailable) {
            QTimer::singleShot(3000, [=]{
                restart();
                retryAvailable = true;
            });
            retryAvailable = false;
        }
        qWarning() << "No connection with Objectwheel Previewing Service";
    }
}

void PreviewerBackend::disableDirtHandling()
{
    s_dirtHandlingEnabled = false;
}

void PreviewerBackend::enableDirtHandling()
{
    s_dirtHandlingEnabled = true;
    s_dirtHandlingDisablerTimer->start();
}

void PreviewerBackend::requestInit(const QString& projectDir)
{
    if (s_disabled)
        return;

    Task task;
    task.dir = projectDir;
    task.uid = "initialization";
    task.type = Task::Init;

    if (!s_taskList.contains(task)) {
        s_taskList << task;

        if (s_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, instance(), &PreviewerBackend::busyChanged);
        }
    }
}

void PreviewerBackend::requestPreview(const QString& dir, bool repreview)
{
    if (s_disabled)
        return;

    Task task;
    task.dir = dir;
    task.uid = SaveUtils::uid(dir);
    task.type = repreview ? Task::Repreview : Task::Preview;

    if (!s_taskList.contains(task)) {
        s_taskList << task;

        if (s_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, instance(), &PreviewerBackend::busyChanged);
        }
    }
}

void PreviewerBackend::removeCache(const QString& uid)
{
    if (s_disabled)
        return;

    Task task;
    task.uid = uid;
    task.type = Task::Remove;

    if (!s_taskList.contains(task)) {
        s_taskList << task;

        if (s_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, instance(), &PreviewerBackend::busyChanged);
        }
    }
}

void PreviewerBackend::updateParent(const QString& uid, const QString& parentUid, const QString& newUrl)
{
    if (s_disabled)
        return;

    fixTasksAgainstReparent(uid, newUrl);

    Task task;
    task.uid = uid;
    task.newUrl = newUrl;
    task.parentUid = parentUid;
    task.type = Task::Reparent;

    if (!s_taskList.contains(task)) {
        s_taskList << task;

        if (s_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, instance(), &PreviewerBackend::busyChanged);
        }
    } else {
        int index = s_taskList.indexOf(task);

        s_taskList[index].newUrl = newUrl;
        s_taskList[index].parentUid = parentUid;

        if (index == 0)
            s_taskList[index].needsUpdate = true;
    }
}

void PreviewerBackend::updateCache(const QString& uid, const QString& property, const QVariant& value)
{
    if (s_disabled)
        return;

    Task task;
    task.uid = uid;
    task.property = property;
    task.propertyValue = value;
    task.type = Task::Update;

    if (!s_taskList.contains(task)) {
        s_taskList << task;

        if (s_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, instance(), &PreviewerBackend::busyChanged);
        }
    } else {
        int index = s_taskList.indexOf(task);

        s_taskList[index].property = property;
        s_taskList[index].propertyValue = value;

        if (index == 0)
            s_taskList[index].needsUpdate = true;
    }
}

void PreviewerBackend::fixTasksAgainstReparent(const QString& uid, const QString& newUrl)
{
    for (auto& task : s_taskList) {
        if (task.uid == uid) {
            task.dir = dname(dname(newUrl));
        }
    }
}

void PreviewerBackend::onNewConnection()
{
    auto client = s_server->nextPendingConnection();

    if (socket.isNull()) {
        socket = client;

        connect(socket.data(), &QLocalSocket::disconnected, socket.data(), &QLocalSocket::deleteLater);
        connect(socket.data(), &QLocalSocket::readyRead, &PreviewerBackend::onReadReady);

        processNextTask();
    } else {
        terminate(client);
    }
}

void PreviewerBackend::onReadReady()
{
    if (socket) {
        QDataStream in(socket);

        if (blockSize == 0) {
            if (socket->bytesAvailable() < (int)sizeof(quint32))
                return;

            in >> blockSize;
        }

        if (socket->bytesAvailable() < blockSize)
            return;

        if (in.atEnd()) {
            blockSize = 0;
            return;
        }

        QByteArray data;
        in >> data;

        emit onBinaryMessageReceived(data);

        blockSize = 0;
    }
}

void PreviewerBackend::onBinaryMessageReceived(const QByteArray& data)
{
    QDataStream in(data);
    QString type;
    in >> type;
    processMessage(type, in);
}

void PreviewerBackend::processNextTask()
{
    if (!s_taskList.isEmpty() && socket) {
        const auto& task = s_taskList.first();

        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);

        if (task.type == Task::Init) {
            out << REQUEST_INIT;
            out << task.dir;
        } else if (task.type == Task::Preview) {
            out << REQUEST_PREVIEW;
            out << task.dir;
        } else if (task.type == Task::Repreview) {
            out << REQUEST_REPREVIEW;
            out << task.dir;
            enableDirtHandling();
        } else if (task.type == Task::Reparent) {
            out << REQUEST_REPARENT;
            out << task.uid;
            out << task.parentUid;
            out << task.newUrl;
            // We don't need cause we get an position property update with reparent in anyways
            // enableDirtHandling();
        } else if (task.type == Task::Remove) {
            out << REQUEST_REMOVE;
            out << task.uid;
            enableDirtHandling();
        } else if (task.type == Task::Update) {
            out << REQUEST_UPDATE;
            out << task.uid;
            out << task.property;
            out << task.propertyValue;
            enableDirtHandling();
        }

        send(socket.data(), data);
    }
}

void PreviewerBackend::processMessage(const QString& type, QDataStream& in)
{
    if (type == REQUEST_DONE) {
        Task::Type t = s_taskList.first().type;

        if (!s_taskList.first().needsUpdate) {
            s_taskList.removeFirst();
            QTimer::singleShot(0, instance(), &PreviewerBackend::taskDone);
        } else
            s_taskList.first().needsUpdate = false;

        PreviewResult result;

        if (t == Task::Remove || t == Task::Preview || t == Task::Repreview || t == Task::Update)
            in >> result;

        if (t == Task::Remove || t == Task::Repreview || t == Task::Update) {
            if (s_dirtHandlingEnabled) {
                for (auto control : Control::controls()) {
                    if (result.dirtyUids.contains(control->uid())) {
                        Task task;
                        task.dir = control->dir();
                        task.uid = control->uid();
                        task.type = Task::Preview;

                        if (!s_taskList.contains(task))
                            s_taskList << task;
                    }
                }
            }
        }

        if (t == Task::Preview || t == Task::Repreview)
            QTimer::singleShot(0, std::bind(&PreviewerBackend::previewReady, instance(), result));

        if (!isBusy())
            QTimer::singleShot(0, instance(), &PreviewerBackend::busyChanged);

        return processNextTask();
    }

    qWarning() << tr("Unknown message arrived.");
}
