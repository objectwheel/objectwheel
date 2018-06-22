#include <controlpreviewingmanager.h>
#include <projectmanager.h>
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
    out << TERMINATE;
    send(socket, data);
    socket->abort();
    ::socket = nullptr;
}

void restart(QLocalSocket* socket, const QStringList& arguments)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << RESTART;
    out << arguments;
    send(socket, data);
    socket->abort();
    ::socket = nullptr;
}
}

ControlPreviewingManager* ControlPreviewingManager::s_instance = nullptr;
bool ControlPreviewingManager::s_disabled = false;
bool ControlPreviewingManager::s_dirtHandlingEnabled = false;
QLocalServer* ControlPreviewingManager::s_server = nullptr;
QTimer* ControlPreviewingManager::s_dirtHandlingDisablerTimer = nullptr;
QList<ControlPreviewingManager::Task> ControlPreviewingManager::s_taskList;

ControlPreviewingManager::ControlPreviewingManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_server = new QLocalServer(this);
    s_dirtHandlingDisablerTimer = new QTimer(this);

    s_server->setSocketOptions(QLocalServer::UserAccessOption);
    connect(s_server, &QLocalServer::newConnection, &ControlPreviewingManager::onNewConnection);
    serverName = HashFactory::generate();
    s_dirtHandlingDisablerTimer->setInterval(500);
    connect(s_dirtHandlingDisablerTimer, &QTimer::timeout, &ControlPreviewingManager::disableDirtHandling);
}

ControlPreviewingManager::~ControlPreviewingManager()
{
    if (socket)
        terminate(socket);
    s_instance = nullptr;
}

ControlPreviewingManager* ControlPreviewingManager::instance()
{
    return s_instance;
}

bool ControlPreviewingManager::init()
{
#if defined(PREVIEWER_DEBUG)
    QLocalServer::removeServer("serverName");
    return s_server->listen("serverName");
#else
    QProcess process;
    process.setProgram(qApp->applicationDirPath() + "/previewer");
    process.setArguments(QStringList() << serverName);
    // process.setStandardOutputFile(QProcess::nullDevice());
    // process.setStandardErrorFile(QProcess::nullDevice());
    return s_server->listen(serverName) && process.startDetached();
#endif
}

bool ControlPreviewingManager::isBusy()
{
    return !s_taskList.isEmpty();
}

int ControlPreviewingManager::totalTask()
{
    return s_taskList.size();
}

void ControlPreviewingManager::setDisabled(bool value)
{
    s_disabled = value;
}

bool ControlPreviewingManager::contains(const QString& uid)
{
    for (const auto& task : s_taskList)
        if (task.uid == uid)
            return true;
    return false;
}

void ControlPreviewingManager::restart()
{
    if (socket && s_taskList.isEmpty()) {
        blockSize = 0;
#if !defined(PREVIEWER_DEBUG)
        ::restart(
                    socket.data(),
                    QStringList()
                    << ProjectManager::dir()
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

void ControlPreviewingManager::disableDirtHandling()
{
    s_dirtHandlingEnabled = false;
}

void ControlPreviewingManager::enableDirtHandling()
{
    s_dirtHandlingEnabled = true;
    s_dirtHandlingDisablerTimer->start();
}

void ControlPreviewingManager::requestInit(const QString& projectDir)
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
            QTimer::singleShot(0, instance(), &ControlPreviewingManager::busyChanged);
        }
    }
}

void ControlPreviewingManager::requestPreview(const QString& dir, bool repreview)
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
            QTimer::singleShot(0, instance(), &ControlPreviewingManager::busyChanged);
        }
    }
}

void ControlPreviewingManager::removeCache(const QString& uid)
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
            QTimer::singleShot(0, instance(), &ControlPreviewingManager::busyChanged);
        }
    }
}

void ControlPreviewingManager::updateParent(const QString& uid, const QString& parentUid, const QString& newUrl)
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
            QTimer::singleShot(0, instance(), &ControlPreviewingManager::busyChanged);
        }
    } else {
        int index = s_taskList.indexOf(task);

        s_taskList[index].newUrl = newUrl;
        s_taskList[index].parentUid = parentUid;

        if (index == 0)
            s_taskList[index].needsUpdate = true;
    }
}

void ControlPreviewingManager::updateCache(const QString& uid, const QString& property, const QVariant& value)
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
            QTimer::singleShot(0, instance(), &ControlPreviewingManager::busyChanged);
        }
    } else {
        int index = s_taskList.indexOf(task);

        s_taskList[index].property = property;
        s_taskList[index].propertyValue = value;

        if (index == 0)
            s_taskList[index].needsUpdate = true;
    }
}

void ControlPreviewingManager::fixTasksAgainstReparent(const QString& uid, const QString& newUrl)
{
    for (auto& task : s_taskList) {
        if (task.uid == uid) {
            task.dir = dname(dname(newUrl));
        }
    }
}

void ControlPreviewingManager::onNewConnection()
{
    auto client = s_server->nextPendingConnection();

    if (socket.isNull()) {
        socket = client;

        connect(socket.data(), &QLocalSocket::disconnected, socket.data(), &QLocalSocket::deleteLater);
        connect(socket.data(), &QLocalSocket::readyRead, &ControlPreviewingManager::onReadReady);

        processNextTask();
    } else {
        terminate(client);
    }
}

void ControlPreviewingManager::onReadReady()
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

void ControlPreviewingManager::onBinaryMessageReceived(const QByteArray& data)
{
    QDataStream in(data);
    int type;
    in >> type;
    processMessage(type, in);
}

void ControlPreviewingManager::processNextTask()
{
    if (!s_taskList.isEmpty() && socket) {
        const auto& task = s_taskList.first();

        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);

        if (task.type == Task::Init) {
            out << INIT;
            out << task.dir;
        } else if (task.type == Task::Preview) {
            out << PREVIEW;
            out << task.dir;
        } else if (task.type == Task::Repreview) {
            out << REPREVIEW;
            out << task.dir;
            enableDirtHandling();
        } else if (task.type == Task::Reparent) {
            out << REPARENT;
            out << task.uid;
            out << task.parentUid;
            out << task.newUrl;
            // We don't need cause we get an position property update with reparent in anyways
            // enableDirtHandling();
        } else if (task.type == Task::Remove) {
            out << REMOVE;
            out << task.uid;
            enableDirtHandling();
        } else if (task.type == Task::Update) {
            out << UPDATE;
            out << task.uid;
            out << task.property;
            out << task.propertyValue;
            enableDirtHandling();
        }

        send(socket.data(), data);
    }
}

void ControlPreviewingManager::processMessage(int type, QDataStream& in)
{
    if (type == DONE) {
        Task::Type t = s_taskList.first().type;

        if (!s_taskList.first().needsUpdate) {
            s_taskList.removeFirst();
            QTimer::singleShot(0, instance(), &ControlPreviewingManager::taskDone);
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
            QTimer::singleShot(0, std::bind(&ControlPreviewingManager::previewReady, instance(), result));

        if (!isBusy())
            QTimer::singleShot(0, instance(), &ControlPreviewingManager::busyChanged);

        return processNextTask();
    }

    qWarning() << tr("Unknown message arrived.");
}
