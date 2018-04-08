#include <previewerbackend.h>
#include <projectbackend.h>
#include <previewresult.h>
#include <previewercommands.h>
#include <delayer.h>
#include <saveutils.h>
#include <control.h>
#include <hashfactory.h>

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

PreviewerBackend::PreviewerBackend() : m_disabled(false)
  , m_server(new QLocalServer(this))
  , m_dirtHandlingDisablerTimer(new QTimer(this))
  , m_dirtHandlingEnabled(false)
{
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    connect(m_server, SIGNAL(newConnection()), SLOT(onNewConnection()));
    serverName = HashFactory::generate();
    m_dirtHandlingDisablerTimer->setInterval(500);
    connect(m_dirtHandlingDisablerTimer, SIGNAL(timeout()), SLOT(disableDirtHandling()));
}

PreviewerBackend::~PreviewerBackend()
{
    if (socket)
        terminate(socket);
}

PreviewerBackend* PreviewerBackend::instance()
{
    static PreviewerBackend instance;
    return &instance;
}

bool PreviewerBackend::init()
{
    #if defined(PREVIEWER_DEBUG)
      QLocalServer::removeServer("serverName");
      return m_server->listen("serverName");
    #else
      QProcess process;
      process.setProgram(qApp->applicationDirPath() + "/objectwheel-previewer");
      process.setArguments(QStringList() << serverName);
      // process.setStandardOutputFile(QProcess::nullDevice());
      // process.setStandardErrorFile(QProcess::nullDevice());
      return m_server->listen(serverName) && process.startDetached();
    #endif
}

bool PreviewerBackend::isBusy() const
{
    return !m_taskList.isEmpty();
}

int PreviewerBackend::totalTask() const
{
    return m_taskList.size();
}

void PreviewerBackend::setDisabled(bool value)
{
    m_disabled = value;
}

bool PreviewerBackend::contains(const QString& uid) const
{
    for (const auto& task : m_taskList)
        if (task.uid == uid)
            return true;
    return false;
}

void PreviewerBackend::restart()
{
    if (socket && m_taskList.isEmpty()) {
        blockSize = 0;        
        #if !defined(PREVIEWER_DEBUG)
          ::restart(
              socket.data(),
              QStringList() << ProjectBackend::instance()->dir() << serverName
          );
        #endif
    } else {
        qFatal("No connection with Objectwheel Previewing Service");
    }
}

void PreviewerBackend::disableDirtHandling()
{
    m_dirtHandlingEnabled = false;
}

void PreviewerBackend::enableDirtHandling()
{
    m_dirtHandlingEnabled = true;
    m_dirtHandlingDisablerTimer->start();
}

void PreviewerBackend::requestInit(const QString& projectDir)
{
    if (m_disabled)
        return;

    Task task;
    task.dir = projectDir;
    task.uid = "initialization";
    task.type = Task::Init;

    if (!m_taskList.contains(task)) {
        m_taskList << task;

        if (m_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, this, &PreviewerBackend::busyChanged);
        }
    }
}

void PreviewerBackend::requestPreview(const QSizeF& size, const QString& dir, bool repreview)
{
    if (m_disabled)
        return;

    Task task;
    task.dir = dir;
    task.uid = SaveUtils::uid(dir);
    task.type = repreview ? Task::Repreview : Task::Preview;
    task.size = size;

    if (!m_taskList.contains(task)) {
        m_taskList << task;

        if (m_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, this, &PreviewerBackend::busyChanged);
        }
    } else {
        int index = m_taskList.indexOf(task);

        m_taskList[index].size = size;

        if (index == 0)
            m_taskList[index].needsUpdate = true;
    }
}

void PreviewerBackend::removeCache(const QString& uid)
{
    if (m_disabled)
        return;

    Task task;
    task.uid = uid;
    task.type = Task::Remove;

    if (!m_taskList.contains(task)) {
        m_taskList << task;

        if (m_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, this, &PreviewerBackend::busyChanged);
        }
    }
}

void PreviewerBackend::updateParent(const QString& uid, const QString& parentUid, const QString& newUrl)
{
    if (m_disabled)
        return;

    Task task;
    task.uid = uid;
    task.newUrl = newUrl;
    task.parentUid = parentUid;
    task.type = Task::Reparent;

    if (!m_taskList.contains(task)) {
        m_taskList << task;

        if (m_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, this, &PreviewerBackend::busyChanged);
        }
    } else {
        int index = m_taskList.indexOf(task);

        m_taskList[index].newUrl = newUrl;
        m_taskList[index].parentUid = parentUid;

        if (index == 0)
            m_taskList[index].needsUpdate = true;
    }
}

void PreviewerBackend::updateCache(const QString& uid, const QString& property, const QVariant& value)
{
    if (m_disabled)
        return;

    Task task;
    task.uid = uid;
    task.property = property;
    task.propertyValue = value;
    task.type = Task::Update;

    if (!m_taskList.contains(task)) {
        m_taskList << task;

        if (m_taskList.size() == 1) {
            processNextTask();
            QTimer::singleShot(0, this, &PreviewerBackend::busyChanged);
        }
    } else {
        int index = m_taskList.indexOf(task);

        m_taskList[index].property = property;
        m_taskList[index].propertyValue = value;

        if (index == 0)
            m_taskList[index].needsUpdate = true;
    }
}

void PreviewerBackend::onNewConnection()
{
    auto client = m_server->nextPendingConnection();

    if (socket.isNull()) {
        socket = client;

        connect(socket.data(), SIGNAL(disconnected()), socket.data(), SLOT(deleteLater()));
        connect(socket.data(), SIGNAL(readyRead()), SLOT(onReadReady()));

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
    if (!m_taskList.isEmpty() && socket) {
        const auto& task = m_taskList.first();

        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);

        if (task.type == Task::Init) {
            out << REQUEST_INIT;
            out << task.dir;
        } else if (task.type == Task::Preview) {
            out << REQUEST_PREVIEW;
            out << task.size;
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
            enableDirtHandling();
        } else if (task.type == Task::Remove) {
            out << REQUEST_REMOVE;
            out << task.uid;
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
        Task::Type t = m_taskList.first().type;

        if (!m_taskList.first().needsUpdate) {
            m_taskList.removeFirst();
            QTimer::singleShot(0, this, &PreviewerBackend::taskDone);
        } else
            m_taskList.first().needsUpdate = false;

        if (t == Task::Preview || t == Task::Repreview) {
            PreviewResult result;
            in >> result;

            if (m_dirtHandlingEnabled) {
                for (auto control : Control::controls()) {
                    if (result.dirtyUids.contains(control->uid())) {
                        Task task;
                        task.dir = control->dir();
                        task.uid = control->uid();
                        task.size = control->size();
                        task.type = Task::Preview;

                        if (!m_taskList.contains(task)) {
                            m_taskList << task;
                        } else {
                            int index = m_taskList.indexOf(task);

                            if (index == 0)
                                m_taskList[index].needsUpdate = true;
                        }
                    }
                }
            }

            QTimer::singleShot(0, std::bind(&PreviewerBackend::previewReady, this, result));
        }

        if (!isBusy())
            QTimer::singleShot(0, this, &PreviewerBackend::busyChanged);

        return processNextTask();
    }

    qWarning() << tr("Unknown message arrived.");
}
