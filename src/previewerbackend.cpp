#include <previewerbackend.h>
#include <projectbackend.h>
#include <previewresult.h>
#include <previewercommands.h>
#include <delayer.h>

#include <QProcess>
#include <QLocalSocket>
#include <QDataStream>

namespace {
    quint32 blockSize = 0;
}

PreviewerBackend::PreviewerBackend()
{
    _process = new QProcess(this);
    _socket = new QLocalSocket(this);
    connect(_socket, SIGNAL(readyRead()), SLOT(onReadReady()));
}

PreviewerBackend* PreviewerBackend::instance()
{
    static PreviewerBackend instance;
    return &instance;
}

bool PreviewerBackend::isWorking() const
{
    return !_taskList.isEmpty();
}

void PreviewerBackend::start()
{
    _process->setArguments(QStringList() << ProjectBackend::instance()->dir());
    _process->setProgram("./objectwheel-previewer");
//    _process->start();

//    _process->waitForStarted();

    connect();
}

void PreviewerBackend::restart()
{
    _process->kill();
    _process->waitForFinished();
    start();
}

void PreviewerBackend::requestPreview(const QRectF& rect, const QString& dir)
{
    Task task;
    task.dir = dir;
    task.rect = rect;
    task.repreview = false;

    if (!_taskList.contains(task)) {
        _taskList << task;

        if (_taskList.size() == 1)
            next();
    } else {
        _taskList[_taskList.indexOf(task)].rect = task.rect;
        _taskList[_taskList.indexOf(task)].repreview = task.repreview;
    }
}

void PreviewerBackend::requestRepreview(const QRectF& rect, const QString& dir)
{
    Task task;
    task.dir = dir;
    task.rect = rect;
    task.repreview = true;

    if (!_taskList.contains(task)) {
        _taskList << task;

        if (_taskList.size() == 1)
            next();
    } else {
        _taskList[_taskList.indexOf(task)].rect = task.rect;
        _taskList[_taskList.indexOf(task)].repreview = task.repreview;
    }
}

void PreviewerBackend::next()
{
    if (!_taskList.isEmpty()) {
        const auto& task = _taskList.first();

        qDebug() << task.dir;

        QByteArray data, msg;
        QDataStream dataStream(&data, QIODevice::WriteOnly);
        QDataStream out(&msg, QIODevice::WriteOnly);
        if (task.repreview) {            
            dataStream << REQUEST_REPREVIEW;
            dataStream << task.rect;
            dataStream << task.dir;

            out << quint32(data.size());
            out << data;
        } else {
            dataStream << REQUEST_PREVIEW;
            dataStream << task.rect;
            dataStream << task.dir;

            out << quint32(data.size());
            out << data;
        }

        _socket->write(msg);
    }
}

void PreviewerBackend::connect()
{
    if (_socket->state() == QLocalSocket::ConnectedState)
        return;

    _socket->connectToServer(SERVER_NAME);

//    Delayer::delay([=] () -> bool {
//        return _socket->state() == QLocalSocket::ConnectedState;
//    }, true, 10000, 2000);

//    if (_socket->state() != QLocalSocket::ConnectedState)
//        _socket->close();
}

void PreviewerBackend::onReadReady()
{
    QDataStream in(_socket);

    if (blockSize == 0) {
        if (_socket->bytesAvailable() < (int)sizeof(quint32))
            return;

        in >> blockSize;
    }

    if (_socket->bytesAvailable() < blockSize)
        return;

    if (in.atEnd()) {
        blockSize = 0;
        return;
    }

    QByteArray data;
    in >> data;

    emit onBinaryMessageReceived(data);

    _taskList.removeFirst();
    blockSize = 0;

    next();
}

void PreviewerBackend::onBinaryMessageReceived(const QByteArray& data)
{
    QDataStream in(data);

    QString type;
    in >> type;

    processMessage(type, in);
}

void PreviewerBackend::processMessage(const QString& type, QDataStream& in)
{
    if (type == REQUEST_READY) {
        PreviewResult result;
        in >> result;

        emit previewReady(result);
    }
}

