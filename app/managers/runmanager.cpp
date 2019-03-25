#include <runmanager.h>
#include <zipasync.h>
#include <saveutils.h>
#include <utilityfunctions.h>
#include <async.h>

#include <QUdpSocket>
#include <QWebSocketServer>
#include <QTimerEvent>
#include <QTemporaryDir>
#include <QTimer>
#include <QApplication>
#include <QPointer>

// TODO: Add encryption (wss)

RunManager* RunManager::s_instance = nullptr;
QBasicTimer RunManager::s_broadcastTimer;
QUdpSocket* RunManager::s_broadcastSocket = nullptr;
QWebSocketServer* RunManager::s_webSocketServer = nullptr;
RunManager::UploadOperation* RunManager::s_uploadOperation = nullptr;
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

    connect(localDevice.process, &QProcess::readyReadStandardError,
            this, [=] { emit processReadyOutput(localDevice.process->readAllStandardError()); });
    connect(localDevice.process, &QProcess::readyReadStandardOutput,
            this, [=] { emit processReadyOutput(localDevice.process->readAllStandardOutput()); });
    connect(localDevice.process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &RunManager::processFinished);
    connect(localDevice.process, &QProcess::errorOccurred, this, [=] (QProcess::ProcessError error)
    { emit processErrorOccurred(error, localDevice.process->errorString()); });
    connect(localDevice.process, &QProcess::started, this, &RunManager::processStarted);

    connect(qApp, &QApplication::aboutToQuit, this, &RunManager::terminate);

    s_webSocketServer->listen(QHostAddress::Any, SERVER_PORT);
    s_broadcastTimer.start(2000, this);
    QTimer::singleShot(0, [=] { deviceConnected(localDevice.uid()); });
}

RunManager::~RunManager()
{
    cleanUploadOperationCache();
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

void RunManager::terminate()
{
    cleanUploadOperationCache();
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

void RunManager::execute(const QString& uid, const QString& projectDirectory)
{
    if (const Device& device = Device::get(s_devices, uid)) {
        terminate();
        s_recentDeviceUid = uid;
        if (isLocalDevice(uid)) {
            device.process->setArguments(QStringList(projectDirectory));
            device.process->setProgram(QCoreApplication::applicationDirPath() + "/interpreter");
            device.process->start();
        } else {
            s_uploadOperation = new UploadOperation;
            if (!s_uploadOperation->cacheDir.isValid()) {
                emit instance()->deviceErrorOccurred(QT_TR_NOOP("Cannot create a temporary directory"));
                QMetaObject::invokeMethod(instance(), &RunManager::cleanUploadOperationCache);
                return;
            }

            const QString& cacheFileName = s_uploadOperation->cacheDir.path() + "/project.zip";

            QObject::connect(&s_uploadOperation->zipWatcher, &QFutureWatcherBase::progressValueChanged, [] (int p)
            {
                emit instance()->deviceUploadProgress(p / 3);
                sendProgressReport(p);
            });

            QObject::connect(&s_uploadOperation->zipWatcher, &QFutureWatcherBase::finished, [=]
            {
                if (s_uploadOperation->zipWatcher.future().resultCount() > 0) {
                    int last = s_uploadOperation->zipWatcher.future().resultCount() - 1;
                    size_t result = s_uploadOperation->zipWatcher.resultAt(last);
                    if (result == 0) {
                        emit instance()->deviceErrorOccurred(s_uploadOperation->zipWatcher.progressText());
                        QMetaObject::invokeMethod(instance(), &RunManager::cleanUploadOperationCache);
                    } else if (!s_uploadOperation->zipWatcher.isCanceled()) {
                        s_uploadOperation->uploadWatcher.setFuture(Async::run([=] (QFutureInterfaceBase* futureInterface) {
                            auto future = static_cast<QFutureInterface<void>*>(futureInterface);
                            QFile cacheFile(cacheFileName);
                            if (!cacheFile.open(QFile::ReadOnly)) {
                                QMetaObject::invokeMethod(instance(), "deviceErrorOccurred", Qt::AutoConnection,
                                                          Q_ARG(QString, QT_TR_NOOP("annot open a temporary file")));
                                QMetaObject::invokeMethod(instance(), &RunManager::cleanUploadOperationCache);
                                return;
                            }
                            const int CHUNK_SIZE = 40960;
                            for (qint64 i = 0; i < cacheFile.size(); i += CHUNK_SIZE) {
                                if (future->isProgressUpdateNeeded() && future->isCanceled())
                                    return;

                                bool eof = cacheFile.size() - i <= CHUNK_SIZE;
                                int progress = 100 * i / cacheFile.size();
                                const QString& projectUid = eof
                                        ? SaveUtils::projectUid(projectDirectory)
                                        : QString();
                                cacheFile.seek(i);
                                const QByteArray& chunk = cacheFile.read(CHUNK_SIZE);
                                QMetaObject::invokeMethod(instance(), "deviceUploadProgress", Qt::AutoConnection,
                                                          Q_ARG(int, 33 + progress / 3));
                                QMetaObject::invokeMethod(instance(),
                                [=] { send(uid, Execute, projectUid, progress, i, chunk); });
                                QThread::msleep(5);
                            }
                        }));
                    }
                }
            });

            s_uploadOperation->zipWatcher.setFuture(ZipAsync::zip(projectDirectory, cacheFileName));

            if (s_uploadOperation->zipWatcher.isCanceled()) {
                emit instance()->deviceErrorOccurred(QT_TR_NOOP("Cannot create a zip archive"));
                QMetaObject::invokeMethod(instance(), &RunManager::cleanUploadOperationCache);
                return;
            }

            sendUploadStarted();
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
    if (const Device& device = Device::get(s_devices, client)) {
        const QString& uid = device.uid();
        s_devices.removeOne(device);
        client->deleteLater();
        emit deviceDisconnected(uid);
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

void RunManager::cleanUploadOperationCache()
{
    if (s_uploadOperation) {
        if (!s_uploadOperation->uploadWatcher.isFinished()) {
            s_uploadOperation->uploadWatcher.cancel();
            s_uploadOperation->uploadWatcher.waitForFinished();
        }
        if (!s_uploadOperation->zipWatcher.isFinished()) {
            s_uploadOperation->zipWatcher.cancel();
            s_uploadOperation->zipWatcher.waitForFinished();
        }
        delete s_uploadOperation;
    }
    s_uploadOperation = nullptr;
}
