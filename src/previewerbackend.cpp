#include <previewerbackend.h>
#include <projectbackend.h>
#include <previewresult.h>
#include <previewercommands.h>
#include <delayer.h>

#include <QProcess>
#include <QLocalSocket>

namespace {
    quint32 blockSize = 0;
}

PreviewerBackend::PreviewerBackend()
{
    _process = new QProcess(this);
    _socket = new QLocalSocket(this);
    connect(_socket, SIGNAL(readyRead()), SLOT(onReadReady()));

    stream.setDevice(_socket);
    stream.setVersion(QDataStream::Qt_5_10);
}

PreviewerBackend* PreviewerBackend::instance()
{
    static PreviewerBackend instance;
    return &instance;
}

bool PreviewerBackend::isWorking() const
{
    return _isWorking;
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
    stream << quint32(SIZE_MSGTYPE) + quint32(sizeof(QRectF)) + quint32(dir.size());
    stream << REQUEST_PREVIEW;
    stream << rect;
    stream << dir;
}

void PreviewerBackend::connect()
{
    if (_socket->state() == QLocalSocket::ConnectedState)
        return;

    _socket->connectToServer(SERVER_NAME);

//    Delayer::delay([=] () -> bool {
//        qDebug() << _socket->state();
//        return _socket->state() == QLocalSocket::ConnectedState;
//    }, true, 10000, 2000);

//    if (_socket->state() != QLocalSocket::ConnectedState)
//        _socket->close();
}

void PreviewerBackend::onReadReady()
{
    if (blockSize == 0) {
        // Relies on the fact that QDataStream serializes a quint32 into
        // sizeof(quint32) bytes
        if (_socket->bytesAvailable() < (int)sizeof(quint32))
            return;

        stream >> blockSize;
    }

    if (_socket->bytesAvailable() < blockSize)
        return;

    if (stream.atEnd()) {
        blockSize = 0;
        return;
    }

    QString type;
    stream >> type;

    processMessage(type);

    blockSize = 0;
}

void PreviewerBackend::processMessage(const QString& type)
{
    if (type == REQUEST_READY) {
        PreviewResult result;
        stream >> result;

        emit previewReady(result);
    }
}

