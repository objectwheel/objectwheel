#include <previewerbackend.h>
#include <projectbackend.h>
#include <previewresult.h>
#include <previewercommands.h>
#include <delayer.h>

#include <QCryptographicHash>
#include <QLocalSocket>
#include <QDataStream>
#include <QMessageBox>
#include <QApplication>
#include <QDateTime>

namespace {
    quint32 blockSize = 0;
    QString serverName;
}

PreviewerBackend::PreviewerBackend()
{
    _process = new QProcess(this);
    _socket = new QLocalSocket(this);

    connect(_socket, SIGNAL(readyRead()), SLOT(onSocketReadReady()));
    connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)),
      SLOT(onProcessFinish(int, QProcess::ExitStatus)));
    connect(_process, SIGNAL(started()), SLOT(onProcessStart()));

//    connect(_process, &QProcess::readyReadStandardError, [=] {
//        qDebug().noquote().nospace() << _process->readAllStandardError();
//    });

//    connect(_process, &QProcess::readyReadStandardOutput, [=] {
//        qDebug().noquote().nospace() << _process->readAllStandardOutput();
//    });
}

PreviewerBackend* PreviewerBackend::instance()
{
    static PreviewerBackend instance;
    return &instance;
}

bool PreviewerBackend::isReady() const
{
    return _socket->state() == QLocalSocket::ConnectedState;
}

bool PreviewerBackend::isWorking() const
{
    return !_taskList.isEmpty();
}

void PreviewerBackend::start()
{
    serverName = QCryptographicHash::hash(
        QString::number(QDateTime::currentSecsSinceEpoch()).toUtf8(),
        QCryptographicHash::Md5
    ).toHex();

    _process->setProgram("./objectwheel-previewer");
    _process->setArguments(
        QStringList() <<
        ProjectBackend::instance()->dir() <<
        serverName
    );

    _process->start();
    waitForReady();
}

void PreviewerBackend::restart()
{
    _socket->abort();
    _process->kill();
    waitForReady();
}

void PreviewerBackend::requestPreview(const QRectF& rect, const QString& dir, bool repreview)
{
    Task task;
    task.dir = dir;
    task.rect = rect;
    task.repreview = repreview;

    if (!_taskList.contains(task)) {
        _taskList << task;

        if (_taskList.size() == 1) {
            processNextTask();
            emit stateChanged();
        }
    } else {
        _taskList[_taskList.indexOf(task)].rect = task.rect;
        _taskList[_taskList.indexOf(task)].repreview = task.repreview;
    }
}

void PreviewerBackend::onProcessStart()
{
    connectToServer();
}

void PreviewerBackend::onProcessFinish(int, QProcess::ExitStatus)
{
    QTimer::singleShot(0, this, &PreviewerBackend::start);
}

void PreviewerBackend::onSocketReadReady()
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

    emit onSocketBinaryMessageReceived(data);

    blockSize = 0;
}

void PreviewerBackend::onSocketBinaryMessageReceived(const QByteArray& data)
{
    QDataStream in(data);
    QString type;
    in >> type;
    processMessage(type, in);
}

void PreviewerBackend::waitForReady()
{
    Delayer::delay(
        std::bind(
            &PreviewerBackend::isReady,
            PreviewerBackend::instance()
        ),
        true
    );
}

void PreviewerBackend::connectToServer()
{
    if (_socket->state() == QLocalSocket::ConnectedState)
        return;

    _socket->connectToServer(serverName);

    Delayer::delay(
        [=] () -> bool {
            bool connected = _socket->state() == QLocalSocket::ConnectedState;
            if (!connected)
                _socket->connectToServer(serverName);
            return _socket->state() == QLocalSocket::ConnectedState;
        },
        true,
        5000,
        200
    );

    if (_socket->state() != QLocalSocket::ConnectedState) {
        _socket->close();
        qFatal("Unable to start Objectwheel Previewer Layer.");
    }
}

void PreviewerBackend::processNextTask()
{
    if (!_taskList.isEmpty()) {
        const auto& task = _taskList.first();

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

void PreviewerBackend::processMessage(const QString& type, QDataStream& in)
{
    if (type == REQUEST_READY) {
        _taskList.removeFirst();

        PreviewResult result;
        in >> result;
        emit previewReady(result);

        if (!isWorking())
            emit stateChanged();

        QTimer::singleShot(0, this, &PreviewerBackend::processNextTask);

        return;
    }

    qWarning() << tr("Unknown message.");
}
