#include <devicemanager.h>
#include <utilityfunctions.h>

#include <QUdpSocket>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QTimerEvent>
#include <QDataStream>
#include <QJsonObject>

// TODO: Add encryption (wss)

using namespace UtilityFunctions;

const QByteArray DeviceManager::UID_PROPERTY = "__OW_DEVICE_UID__";
const QByteArray DeviceManager::SERVER_NAME = "Objectwheel Device Manager";
DeviceManager* DeviceManager::s_instance = nullptr;
QBasicTimer DeviceManager::s_broadcastTimer;
QUdpSocket* DeviceManager::s_broadcastSocket = nullptr;
QWebSocketServer* DeviceManager::s_webSocketServer = nullptr;
QList<QVariantMap> DeviceManager::s_deviceInfoList;

DeviceManager::DeviceManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_broadcastSocket = new QUdpSocket(this);
    s_webSocketServer = new QWebSocketServer(SERVER_NAME, QWebSocketServer::NonSecureMode, this);
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

void DeviceManager::removeDeviceInfo(const QString& uid)
{
    for (const QVariantMap& info : s_deviceInfoList) {
        if (info.value("deviceUid").toString() == uid) {
            s_deviceInfoList.removeOne(info);
            return;
        }
    }
}

DeviceManager* DeviceManager::instance()
{
    return s_instance;
}

const QList<QVariantMap>& DeviceManager::deviceInfoList()
{
    return s_deviceInfoList;
}

QVariantMap DeviceManager::deviceInfo(const QString& uid)
{
    for (const QVariantMap& info : s_deviceInfoList) {
        if (info.value("deviceUid").toString() == uid)
            return info;
    }
    return QVariantMap();
}

bool DeviceManager::deviceInfoExists(const QString& uid)
{
    for (const QVariantMap& info : s_deviceInfoList) {
        if (info.value("deviceUid").toString() == uid)
            return true;
    }
    return false;
}

void DeviceManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_broadcastTimer.timerId())
        s_broadcastSocket->writeDatagram(BROADCAST_MESSAGE, QHostAddress::Broadcast, BROADCAST_PORT);
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
    removeDeviceInfo(client->property(UID_PROPERTY).toString());
    client->deleteLater();
    emit disconnected(client->property(UID_PROPERTY).toString());
}

void DeviceManager::onBinaryMessageReceived(const QByteArray& incomingData)
{
    QWebSocket* client = static_cast<QWebSocket*>(sender());
    QByteArray data;
    QString command;
    dispatch(incomingData, data, command);

    if (command == "DeviceInfo") {
        QVariantMap info;
        pull(data, info);
        s_deviceInfoList.append(info);
        client->setProperty(UID_PROPERTY, info.value("deviceUid").toString());
        emit connected(info);
    } else {

    }
}
