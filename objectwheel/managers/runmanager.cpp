#include <runmanager.h>
#include <zipasync.h>
#include <saveutils.h>
#include <utilityfunctions.h>
#include <appconstants.h>
#include <applicationcore.h>

#include <QUdpSocket>
#include <QWebSocketServer>
#include <QTimer>
#include <QApplication>
#include <QPointer>
#include <QWebSocket>

// TODO: Add encryption (wss)

RunManager* RunManager::s_instance = nullptr;
QBasicTimer RunManager::s_broadcastTimer;
QBasicTimer RunManager::s_remoteExecutionTimer;
QUdpSocket* RunManager::s_broadcastSocket = nullptr;
QWebSocketServer* RunManager::s_webSocketServer = nullptr;
RunManager::UploadInfo RunManager::s_uploadInfo;
QList<RunManager::Device> RunManager::s_devices;
QString RunManager::s_recentDeviceUid;
QString RunManager::s_recentProjectDirectory;

RunManager::RunManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_broadcastSocket = new QUdpSocket(this);
    s_webSocketServer = new QWebSocketServer(AppConstants::NAME, QWebSocketServer::NonSecureMode, this);

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
            this, [=] { emit applicationReadyOutput(localDevice.process->readAllStandardError()); });
    connect(localDevice.process, &QProcess::readyReadStandardOutput,
            this, [=] { emit applicationReadyOutput(localDevice.process->readAllStandardOutput()); });
    connect(localDevice.process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &RunManager::applicationFinished);
    connect(localDevice.process, &QProcess::errorOccurred, this, [=] (QProcess::ProcessError error)
    { emit applicationErrorOccurred(error, localDevice.process->errorString()); });
    connect(localDevice.process, &QProcess::started, this, &RunManager::applicationStarted);
    connect(qApp, &QApplication::aboutToQuit, this, &RunManager::sendTerminate);

    connect(&s_uploadInfo.watcher, &QFutureWatcherBase::progressValueChanged,
            this, [] (int progress) {
        emit instance()->applicationUploadProgress(progress / 3);
        sendProgressReport(progress);
    });
    connect(&s_uploadInfo.watcher, &QFutureWatcherBase::finished, this, [] {
        if (s_uploadInfo.watcher.future().resultCount() > 0) {
            int lastIndex = s_uploadInfo.watcher.future().resultCount() - 1;
            size_t lastResult = s_uploadInfo.watcher.resultAt(lastIndex);
            if (lastResult == 0) { // Error occurred
                s_uploadInfo.cacheDir.reset(nullptr);
                emit instance()->applicationErrorOccurred(QProcess::FailedToStart,
                    tr(s_uploadInfo.watcher.progressText().toUtf8().data()));
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

QString RunManager::recentProjectDirectory()
{
    return s_recentProjectDirectory;
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
    } else if (event->timerId() == s_remoteExecutionTimer.timerId()) {
        s_remoteExecutionTimer.stop();
        s_uploadInfo.cacheDir.reset(new QTemporaryDir);
        s_uploadInfo.canceled = false;

        if (!s_uploadInfo.cacheDir->isValid()) {
            s_uploadInfo.cacheDir.reset(nullptr);
            emit instance()->applicationErrorOccurred(QProcess::FailedToStart,
                                                      tr("Cannot establish a temporary directory"));
            return;
        }

        s_uploadInfo.fileName = s_uploadInfo.cacheDir->path() + "/project.zip";
        s_uploadInfo.watcher.setFuture(ZipAsync::zip(s_uploadInfo.projectDir, s_uploadInfo.fileName));

        if (s_uploadInfo.watcher.isCanceled()) {
            s_uploadInfo.cacheDir.reset(nullptr);
            emit instance()->applicationErrorOccurred(QProcess::FailedToStart,
                                                      tr("Cannot establish a zip archive"));
            return;
        }

        sendUploadStarted();
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
            s_recentProjectDirectory = projectDirectory;
            device.process->setArguments({projectDirectory});
            device.process->setProgram(ApplicationCore::interpreterPath());
            device.process->start();
        } else {
            s_uploadInfo.deviceUid = uid;
            s_uploadInfo.projectDir = projectDirectory;
            s_remoteExecutionTimer.start(100, instance()); // Wait until ongoing upload cancels
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
        QString projectDirectory;
        UtilityFunctions::pull(data, projectDirectory);
        if (!projectDirectory.isEmpty())
            s_recentProjectDirectory = projectDirectory;
        emit applicationStarted();
        break;
    }

    case OutputReport: {
        QString output;
        UtilityFunctions::pull(data, output);
        emit applicationReadyOutput(output);
        break;
    }

    case ErrorReport: {
        QString errorString;
        UtilityFunctions::pull(data, errorString);
        emit applicationErrorOccurred(QProcess::FailedToStart, errorString);
        break;
    }

    case FinishReport: {
        int exitCode;
        bool crashExit;
        UtilityFunctions::pull(data, exitCode, crashExit);
        if (crashExit)
            emit applicationErrorOccurred(QProcess::Crashed, QString());
        emit applicationFinished(exitCode, crashExit ? QProcess::CrashExit : QProcess::NormalExit);
        break;
    }

    case ProgressReport: {
        int progress;
        UtilityFunctions::pull(data, progress);
        emit applicationUploadProgress(67 + progress / 3);
        break;
    }

    default:
        qWarning("RunManager: Unrecognized command has arrived");
        break;
    }
}

void RunManager::upload()
{
    if (const Device device = Device::get(s_devices, s_uploadInfo.deviceUid)) {
        QFile file(s_uploadInfo.fileName);
        if (!file.open(QFile::ReadOnly)) {
            file.close();
            s_uploadInfo.cacheDir.reset(nullptr);
            emit instance()->applicationErrorOccurred(QProcess::FailedToStart,
                                                      tr("Cannot open a temporary file"));
            return;
        }
        enum { FRAME_SIZE = 10485760 }; // 10MB
        for (qint64 pos = 0; pos < file.size(); pos += FRAME_SIZE) {
            do {
                qApp->processEvents(QEventLoop::AllEvents, 20);
                if (s_uploadInfo.canceled) {
                    file.close();
                    s_uploadInfo.cacheDir.reset(nullptr);
                    return;
                }
            } while(device.socket->bytesToWrite() > 10 * FRAME_SIZE);

            int progress = 100 * pos / file.size();
            bool isLastFrame = file.size() - pos <= FRAME_SIZE;
            const QString& projectUid = isLastFrame ? SaveUtils::projectUid(s_uploadInfo.projectDir) : QString();

            file.seek(pos);
            send(s_uploadInfo.deviceUid, Execute, projectUid, progress, pos, file.read(FRAME_SIZE));
            emit instance()->applicationUploadProgress(33 + progress / 3);
        }
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
