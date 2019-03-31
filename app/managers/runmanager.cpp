#include <runmanager.h>
#include <zipasync.h>
#include <saveutils.h>
#include <utilityfunctions.h>

#include <QUdpSocket>
#include <QWebSocketServer>
#include <QTimer>
#include <QApplication>
#include <QPointer>
#include <QWebSocket>

// TODO: Add encryption (wss)

RunManager* RunManager::s_instance = nullptr;
QBasicTimer RunManager::s_broadcastTimer;
QUdpSocket* RunManager::s_broadcastSocket = nullptr;
QWebSocketServer* RunManager::s_webSocketServer = nullptr;
RunManager::UploadInfo RunManager::s_uploadInfo;
QList<RunManager::Device> RunManager::s_devices;
QString RunManager::s_recentDeviceUid;

RunManager::RunManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_broadcastSocket = new QUdpSocket(this);
    s_webSocketServer = new QWebSocketServer(APP_NAME, QWebSocketServer::NonSecureMode, this);

    connect(s_broadcastSocket, qOverload<QAbstractSocket::SocketError>(&QUdpSocket::error),
            this, [=] (QAbstractSocket::SocketError socketError) {
        qWarning() << "RunManager: Broadcast socket error" << socketError;
    });
    connect(s_webSocketServer, qOverload<QAbstractSocket::SocketError>(&QWebSocketServer::acceptError),
            this, [=] (QAbstractSocket::SocketError socketError) {
        qWarning() << "RunManager: Accept error" << socketError;
    });
    connect(s_webSocketServer, qOverload<QWebSocketProtocol::CloseCode>(&QWebSocketServer::serverError),
            this, [=] (QWebSocketProtocol::CloseCode closeCode) {
        qWarning() << "RunManager: Server error, close code" << closeCode;
    });
    connect(s_webSocketServer, &QWebSocketServer::newConnection,
            this, &RunManager::onNewConnection);

    Device localDevice;
    localDevice.info = UtilityFunctions::localDeviceInfo();
    localDevice.process = new QProcess(this);
    s_recentDeviceUid = localDevice.uid();
    s_devices.append(localDevice);
    QMetaObject::invokeMethod(this, [=] { deviceConnected(localDevice.uid()); }, Qt::QueuedConnection);

    connect(localDevice.process, &QProcess::readyReadStandardError,
            this, [=] { emit processReadyOutput(localDevice.process->readAllStandardError()); });
    connect(localDevice.process, &QProcess::readyReadStandardOutput,
            this, [=] { emit processReadyOutput(localDevice.process->readAllStandardOutput()); });
    connect(localDevice.process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &RunManager::processFinished);
    connect(localDevice.process, &QProcess::errorOccurred, this, [=] (QProcess::ProcessError error)
    { emit processErrorOccurred(error, localDevice.process->errorString()); });
    connect(localDevice.process, &QProcess::started, this, &RunManager::processStarted);
    connect(qApp, &QApplication::aboutToQuit, this, &RunManager::sendTerminate);

    connect(&s_uploadInfo.watcher, &QFutureWatcherBase::progressValueChanged,
            this, [] (int progress) {
        emit instance()->deviceUploadProgress(progress / 3);
        sendProgressReport(progress);
    });
    connect(&s_uploadInfo.watcher, &QFutureWatcherBase::finished, this, [] {
        if (s_uploadInfo.watcher.future().resultCount() > 0) {
            int lastIndex = s_uploadInfo.watcher.future().resultCount() - 1;
            size_t lastResult = s_uploadInfo.watcher.resultAt(lastIndex);
            if (lastResult == 0) { // Error occurred
                s_uploadInfo.cacheDir.reset(nullptr);
                emit instance()->deviceErrorOccurred(tr(s_uploadInfo.watcher.progressText().toUtf8().data()));
            } else if (!s_uploadInfo.watcher.isCanceled()) { // Succeed
                upload();
            }
            // else -> Do nothing, Operation canceled in the middle
        } // else -> Do nothing, operation canceled even before attempting to do anything
    });

    s_webSocketServer->listen(QHostAddress::Any, SERVER_PORT);
    s_broadcastTimer.start(2000, this);
}

RunManager::~RunManager()
{
    scheduleUploadCancelation();
    s_instance = nullptr;
}

RunManager* RunManager::instance()
{
    return s_instance;
}

QString RunManager::recentDevice()
{
    return s_recentDeviceUid;
}

QVariantMap RunManager::deviceInfo(const QString& uid)
{
    return Device::get(s_devices, uid).info;
}

bool RunManager::isLocalDevice(const QString& uid)
{
    return uid == UtilityFunctions::localDeviceInfo().value("deviceUid").toString();
}

template<typename... Args>
void RunManager::send(const QString& uid, RunManager::DiscoveryCommands command, Args&&... args)
{
    if (const Device& device = Device::get(s_devices, uid)) {
        device.socket->sendBinaryMessage(
                    UtilityFunctions::push(command, UtilityFunctions::push(std::forward<Args>(args)...)));
        return;
    }
    qWarning("WARNING: Cannot send any data, device is not connected, uid: %s", uid.toUtf8().constData());
}

void RunManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_broadcastTimer.timerId()) {
        s_broadcastSocket->writeDatagram(UtilityFunctions::push(Broadcast),
                                         QHostAddress::Broadcast, BROADCAST_PORT);
    } else {
        QObject::timerEvent(event);
    }
}

void RunManager::sendTerminate()
{
    scheduleUploadCancelation();
    if (const Device& device = Device::get(s_devices, s_recentDeviceUid)) {
        if (isLocalDevice(s_recentDeviceUid)) {
            if (device.process->state() != QProcess::NotRunning) {
                device.process->kill();
                device.process->waitForFinished(1000);
            }
        } else {
            send(s_recentDeviceUid, Terminate);
        }
    }
}

void RunManager::sendUploadStarted()
{
    send(s_recentDeviceUid, RunManager::UploadStarted);
}

void RunManager::sendProgressReport(int progress)
{
    send(s_recentDeviceUid, RunManager::ProgressReport, progress);
}

void RunManager::sendExecute(const QString& uid, const QString& projectDirectory)
{
    if (const Device& device = Device::get(s_devices, uid)) {
        sendTerminate();
        s_recentDeviceUid = uid;
        if (isLocalDevice(uid)) {
            device.process->setArguments(QStringList(projectDirectory));
            device.process->setProgram(QCoreApplication::applicationDirPath() + "/interpreter");
            device.process->start();
        } else {
            QTimer::singleShot(100, [uid, projectDirectory] { // Wait until ongoing upload cancels
                s_uploadInfo.cacheDir.reset(new QTemporaryDir);
                s_uploadInfo.canceled = false;
                s_uploadInfo.deviceUid = uid;
                s_uploadInfo.projectUid = SaveUtils::projectUid(projectDirectory);

                if (!s_uploadInfo.cacheDir->isValid()) {
                    s_uploadInfo.cacheDir.reset(nullptr);
                    emit instance()->deviceErrorOccurred(tr("Cannot create a temporary directory."));
                    return;
                }

                s_uploadInfo.fileName = s_uploadInfo.cacheDir->path() + "/project.zip";
                s_uploadInfo.watcher.setFuture(ZipAsync::zip(projectDirectory, s_uploadInfo.fileName));

                if (s_uploadInfo.watcher.isCanceled()) {
                    s_uploadInfo.cacheDir.reset(nullptr);
                    emit instance()->deviceErrorOccurred(tr("Cannot create a zip archive."));
                    return;
                }

                sendUploadStarted();
            });
        }
    }
}

void RunManager::onNewConnection()
{
    QWebSocket* client = s_webSocketServer->nextPendingConnection();
    connect(client, &QWebSocket::binaryMessageReceived,
            this, &RunManager::onBinaryMessageReceived);
    connect(client, &QWebSocket::disconnected,
            this, &RunManager::onDisconnected);
    connect(client, qOverload<QAbstractSocket::SocketError>(&QWebSocket::error),
            this, [=] (QAbstractSocket::SocketError socketError) {
        qWarning() << "RunManager: Device socket error" << socketError;
    });
    QPointer<QWebSocket> ptr(client);
    QTimer::singleShot(2000, [ptr] {
        if (ptr && !Device::get(s_devices, ptr.data()))
            ptr->deleteLater();
    });
}

void RunManager::onDisconnected()
{
    QWebSocket* client = static_cast<QWebSocket*>(sender());
    if (const Device device = Device::get(s_devices, client)) {
        s_devices.removeOne(device);
        client->deleteLater();
        emit deviceDisconnected(device.info);
    }
}

void RunManager::onBinaryMessageReceived(const QByteArray& incomingData)
{
    QWebSocket* client = static_cast<QWebSocket*>(sender());
    QByteArray data;
    DiscoveryCommands command;
    UtilityFunctions::pull(incomingData, command, data);

    switch (command) {
    case InfoReport: {
        QVariantMap info;
        UtilityFunctions::pull(data, info);
        Device device;
        device.info = info;
        device.socket = client;
        s_devices.append(device);
        emit deviceConnected(device.uid());
        break;
    }

    case StartReport: {
        emit deviceStarted();
        break;
    }

    case OutputReport: {
        QString output;
        UtilityFunctions::pull(data, output);
        emit deviceReadyOutput(output);
        break;
    }

    case ErrorReport: {
        QString errorString;
        UtilityFunctions::pull(data, errorString);
        emit deviceErrorOccurred(errorString);
        break;
    }

    case FinishReport: {
        int exitCode;
        UtilityFunctions::pull(data, exitCode);
        emit deviceFinished(exitCode);
        break;
    }

    case ProgressReport: {
        int progress;
        UtilityFunctions::pull(data, progress);
        emit deviceUploadProgress(67 + progress / 3);
        break;
    }

    default:
        qWarning("RunManager: Unrecognized command has arrived");
        break;
    }
}

void RunManager::upload()
{
    if (const Device& device = Device::get(s_devices, s_uploadInfo.deviceUid)) {
        QFile file(s_uploadInfo.fileName);
        if (!file.open(QFile::ReadOnly)) {
            file.close();
            s_uploadInfo.cacheDir.reset(nullptr);
            emit instance()->deviceErrorOccurred(tr("Cannot open a temporary file."));
            return;
        }
        auto connection = connect(device.socket, &QWebSocket::bytesWritten, [&file] (qint64 bytes) {
            int progress = 100 * bytes / file.size();
            emit instance()->deviceUploadProgress(33 + progress / 3);
        });
        enum { FRAME_SIZE = 10485760 }; // 10MB
        for (qint64 pos = 0; pos < file.size(); pos += FRAME_SIZE) {
            do {
                qApp->processEvents(QEventLoop::AllEvents, 20);
                if (s_uploadInfo.canceled) {
                    disconnect(connection);
                    file.close();
                    s_uploadInfo.cacheDir.reset(nullptr);
                    return;
                }
            } while(device.socket->bytesToWrite() > 10 * FRAME_SIZE);

            int progress = 100 * pos / file.size();
            bool isLastFrame = file.size() - pos <= FRAME_SIZE;
            const QString& projectUid = isLastFrame ? s_uploadInfo.projectUid : QString();

            file.seek(pos);
            send(s_uploadInfo.deviceUid, Execute, projectUid, progress, pos, file.read(FRAME_SIZE));
        }
        disconnect(connection);
        file.close();
        s_uploadInfo.cacheDir.reset(nullptr);
    }
}

void RunManager::scheduleUploadCancelation()
{
    s_uploadInfo.canceled = true;
    if (!s_uploadInfo.watcher.isFinished()) {
        s_uploadInfo.watcher.cancel();
        s_uploadInfo.watcher.waitForFinished();
    }
}
