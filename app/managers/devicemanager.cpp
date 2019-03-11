#include <devicemanager.h>

#include <QUdpSocket>
#include <QWebSocketServer>
#include <QTimerEvent>
#include <QJsonObject>

// TODO: Add encryption (wss)

using namespace UtilityFunctions;

DeviceManager* DeviceManager::s_instance = nullptr;
QBasicTimer DeviceManager::s_broadcastTimer;
QUdpSocket* DeviceManager::s_broadcastSocket = nullptr;
QWebSocketServer* DeviceManager::s_webSocketServer = nullptr;
QList<DeviceManager::Device> DeviceManager::s_devices;

DeviceManager::DeviceManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_broadcastSocket = new QUdpSocket(this);
    s_webSocketServer = new QWebSocketServer(APP_NAME, QWebSocketServer::NonSecureMode, this);
    s_webSocketServer->listen(QHostAddress::Any, SERVER_PORT);
    s_broadcastTimer.start(1000, this);

    connect(s_broadcastSocket, qOverload<QAbstractSocket::SocketError>(&QUdpSocket::error),
            this, [=] (QAbstractSocket::SocketError socketError) {
        qWarning() << "DeviceManager: Broadcast socket error" << socketError;
    });
    connect(s_webSocketServer, qOverload<QAbstractSocket::SocketError>(&QWebSocketServer::acceptError),
            this, [=] (QAbstractSocket::SocketError socketError) {
        qWarning() << "DeviceManager: Accept error" << socketError;
    });
    connect(s_webSocketServer, qOverload<QWebSocketProtocol::CloseCode>(&QWebSocketServer::serverError),
            this, [=] (QWebSocketProtocol::CloseCode closeCode) {
        qWarning() << "DeviceManager: Server error, close code" << closeCode;
    });
    connect(s_webSocketServer, &QWebSocketServer::newConnection,
            this, &DeviceManager::onNewConnection);
}

DeviceManager::~DeviceManager()
{
    s_instance = nullptr;
}

DeviceManager* DeviceManager::instance()
{
    return s_instance;
}

void DeviceManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_broadcastTimer.timerId())
        s_broadcastSocket->writeDatagram(push(Broadcast), QHostAddress::Broadcast, BROADCAST_PORT);
    else
        QObject::timerEvent(event);
}

void DeviceManager::onNewConnection()
{
    QWebSocket* client = s_webSocketServer->nextPendingConnection();
    connect(client, &QWebSocket::binaryMessageReceived,
            this, &DeviceManager::onBinaryMessageReceived);
    connect(client, &QWebSocket::disconnected,
            this, &DeviceManager::onDisconnected);
    connect(client, qOverload<QAbstractSocket::SocketError>(&QWebSocket::error),
            this, [=] (QAbstractSocket::SocketError socketError) {
        qWarning() << "DeviceManager: Device socket error" << socketError;
    });
}

void DeviceManager::onDisconnected()
{
    QWebSocket* client = static_cast<QWebSocket*>(sender());
    const Device& device = Device::get(s_devices, client);
    s_devices.removeOne(device);
    client->deleteLater();
    emit deviceDisconnected(device.uid());
}

void DeviceManager::onBinaryMessageReceived(const QByteArray& incomingData)
{
    QWebSocket* client = static_cast<QWebSocket*>(sender());
    QByteArray data;
    DiscoveryCommands command;
    pull(incomingData, command, data);

    switch (command) {
    case InfoReport: {
        QVariantMap info;
        pull(data, info);
        Device device;
        device.info = info;
        device.socket = client;
        s_devices.append(device);
        emit deviceConnected(info);
        break;
    }

    case StartReport: {
        emit applicationStarted();
        break;
    }

    case OutputReport: {
        QString output;
        pull(data, output);
        emit applicationReadyReadOutput(output);
        break;
    }

    case FinishReport: {
        int exitCode;
        pull(data, exitCode);
        emit applicationFinished(exitCode);
        break;
    }

    default:
        qWarning("DeviceManager: Unrecognized command has arrived");
        break;
    }
}
