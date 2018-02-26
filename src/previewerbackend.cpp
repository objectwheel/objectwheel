#include <previewerbackend.h>
#include <projectbackend.h>
#include <previewresult.h>
#include <previewercommands.h>
#include <delayer.h>

#include <QProcess>
#include <QLocalSocket>
#include <QDataStream>

PreviewerBackend::PreviewerBackend()
{
    _process = new QProcess(this);
    _socket = new QLocalSocket(this);
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
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);

    out << REQUEST_PREVIEW;
    out << rect;
    out << dir;

    _socket->write(data);
    _socket->flush();
}

void PreviewerBackend::connect()
{
    if (_socket->state() == QLocalSocket::ConnectedState)
        return;

    _socket->connectToServer(SERVER_NAME);

//    Delayer::delay([=] () -> bool {
//        qDebug() << _socket->state();
//        return _socket->state() == QLocalSocket::ConnectedState;
//    }, true);

//    if (_socket->state() != QLocalSocket::ConnectedState)
//        _socket->close();
}

void PreviewerBackend::onReadReady()
{
    if (_socket) {
        const auto& data = _socket->readAll();
        QDataStream in(data);
        QString type;
        in >> type;

        if (type == REQUEST_READY) {
            PreviewResult result;

            in >> result;

            emit previewReady(result);
        }
    }
}

