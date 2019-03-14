#include <runmanager.h>
#include <zipasync.h>
#include <saveutils.h>
#include <utilityfunctions.h>

#include <QUdpSocket>
#include <QWebSocketServer>
#include <QTimerEvent>
#include <QTemporaryFile>
#include <QTimer>
#include <QApplication>

// TODO: Add encryption (wss)

RunManager* RunManager::s_instance = nullptr;
QBasicTimer RunManager::s_broadcastTimer;
QUdpSocket* RunManager::s_broadcastSocket = nullptr;
QWebSocketServer* RunManager::s_webSocketServer = nullptr;
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
            this, [=] { emit projectReadyReadOutput(localDevice.process->readAllStandardError()); });
    connect(localDevice.process, &QProcess::readyReadStandardOutput,
            this, [=] { emit projectReadyReadOutput(localDevice.process->readAllStandardOutput()); });
    connect(localDevice.process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, [=] (int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit)
            emit projectFinished(exitCode);
        else
            emit deviceDisconnected(localDevice.uid());
    });
    connect(localDevice.process, &QProcess::started, this, &RunManager::projectStarted);
    connect(qApp, &QApplication::aboutToQuit, localDevice.process, &QProcess::kill);

    s_webSocketServer->listen(QHostAddress::Any, SERVER_PORT);
    s_broadcastTimer.start(2000, this);
    QTimer::singleShot(5000, [=] { deviceConnected(localDevice.uid()); });
}

RunManager::~RunManager()
{
    s_instance = nullptr;
}

RunManager* RunManager::instance()
{
    return s_instance;
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
            QTemporaryFile cacheFile;
            if (!cacheFile.open()) {
                qFatal("CRITICAL: Cannot create a temporary file");
                return;
            }
            cacheFile.close();
            ZipAsync::zipSync(projectDirectory, cacheFile.fileName());
            if (!cacheFile.open()) {
                qFatal("CRITICAL: Cannot create a temporary file");
                return;
            }
            const QByteArray& data = cacheFile.readAll();
            cacheFile.close();

            const int CHUNK_SIZE = 40960;
            for (qint64 i = 0; i < data.size(); i += CHUNK_SIZE) {
                bool eof = data.size() - i <= CHUNK_SIZE;
                send(uid, Execute, eof
                     ? SaveUtils::projectUid(projectDirectory)
                     : QString(), i, data.mid(i, CHUNK_SIZE));
            }
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
    QTimer::singleShot(2000, [=] { if (!Device::get(s_devices, client)) client->deleteLater(); });
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
        emit projectStarted();
        break;
    }

    case OutputReport: {
        QString output;
        UtilityFunctions::pull(data, output);
        emit projectReadyReadOutput(output);
        break;
    }

    case FinishReport: {
        int exitCode;
        UtilityFunctions::pull(data, exitCode);
        emit projectFinished(exitCode);
        break;
    }

    default:
        qWarning("RunManager: Unrecognized command has arrived");
        break;
    }
}
