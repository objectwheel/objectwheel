#include <previewerbackend.h>
#include <projectbackend.h>
#include <previewresult.h>
#include <previewercommands.h>
#include <delayer.h>
#include <saveutils.h>

#include <QPointer>
#include <QCryptographicHash>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QDateTime>
#include <QProcess>

namespace {
    quint32 blockSize = 0;
    QString serverName;
    QPointer<QLocalSocket> socket;

    typedef QCryptographicHash Hasher;

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

PreviewerBackend::PreviewerBackend()
{
    _server = new QLocalServer(this);
    _server->setSocketOptions(QLocalServer::UserAccessOption);
    connect(_server, SIGNAL(newConnection()), SLOT(onNewConnection()));
    serverName = Hasher::hash(QString::number(QDateTime::currentSecsSinceEpoch()).toUtf8(), Hasher::Md5).toHex();
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
    QProcess process;
    process.setProgram("./objectwheel-previewer");
    process.setArguments(QStringList() << serverName);
//    process.setStandardOutputFile(QProcess::nullDevice());
//    process.setStandardErrorFile(QProcess::nullDevice());
    return _server->listen(serverName) && process.startDetached();
}

bool PreviewerBackend::isBusy() const
{
    return !_taskList.isEmpty();
}

void PreviewerBackend::restart()
{
    if (socket && _taskList.isEmpty()) {
        blockSize = 0;
        ::restart(
            socket.data(),
            QStringList() << ProjectBackend::instance()->dir() << serverName
        );
    } else {
        qFatal("No connection with Objectwheel Previewing Service");
    }
}

void PreviewerBackend::requestPreview(const QRectF& rect, const QString& dir, bool repreview)
{
    Task task;
    task.dir = dir;
    task.rect = rect;
    task.uid = SaveUtils::uid(dir);
    task.type = repreview ? Task::Repreview : Task::Preview;

    if (!_taskList.contains(task)) {
        _taskList << task;

        if (_taskList.size() == 1) {
            processNextTask();
            emit busyChanged();
        }
    } else {
        int index = _taskList.indexOf(task);

        if (index == 0)
            _taskList[index].needsUpdate = true;

        _taskList[index].rect = task.rect;
    }
}

void PreviewerBackend::removeCache(const QString& uid)
{
    Task task;
    task.uid = uid;
    task.type = Task::Remove;

    if (!_taskList.contains(task)) {
        _taskList << task;

        if (_taskList.size() == 1) {
            processNextTask();
            emit busyChanged();
        }
    }
}

void PreviewerBackend::updateCache(const QString& uid, const QString& property, const QVariant& value)
{
    Task task;
    task.uid = uid;
    task.property = property;
    task.propertyValue = value;
    task.type = Task::Update;

    if (!_taskList.contains(task)) {
        _taskList << task;

        if (_taskList.size() == 1) {
            processNextTask();
            emit busyChanged();
        }
    } else {
        int index = _taskList.indexOf(task);

        if (index == 0)
            _taskList[index].needsUpdate = true;

        _taskList[index].property = task.property;
        _taskList[index].propertyValue = task.propertyValue;
    }
}

void PreviewerBackend::onNewConnection()
{
    auto client = _server->nextPendingConnection();

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
    if (!_taskList.isEmpty() && socket) {
        const auto& task = _taskList.first();

        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);

        if (task.type == Task::Preview) {
            out << REQUEST_PREVIEW;
            out << task.rect;
            out << task.dir;
        } else if (task.type == Task::Repreview) {
            out << REQUEST_REPREVIEW;
            out << task.rect;
            out << task.dir;
        } else if (task.type == Task::Remove) {
            out << REQUEST_REMOVE;
            out << task.uid;
        } else if (task.type == Task::Update) {
            out << REQUEST_UPDATE;
            out << task.uid;
            out << task.property;
            out << task.propertyValue;
        }

        send(socket.data(), data);
    }
}

void PreviewerBackend::processMessage(const QString& type, QDataStream& in)
{
    if (type == REQUEST_DONE) {
        Task::Type t = _taskList.first().type;

        if (!_taskList.first().needsUpdate)
            _taskList.removeFirst();
        else
            _taskList.first().needsUpdate = false;

        if (t == Task::Preview || t == Task::Repreview) {
            PreviewResult result;
            in >> result;
            emit previewReady(result);
        }

        if (!isBusy())
            emit busyChanged();

        return processNextTask();
    }

    qWarning() << tr("Unknown message arrived.");
}
