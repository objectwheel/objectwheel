#include <devicemanager.h>
#include <QUdpSocket>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QTimerEvent>
#include <QDataStream>
#include <QJsonObject>

// TODO: Add encryption

namespace {

void pushValuesHelper(QDataStream&) {}

template <typename Arg, typename... Args>
void pushValuesHelper(QDataStream& stream, const Arg& arg, const Args&... args) {
    stream << arg;
    pushValuesHelper(stream, args...);
}

template <typename... Args>
QByteArray pushValues(const Args&... args) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    pushValuesHelper(stream, args...);
    return data;
}

void pullValuesHelper(QDataStream&) {}

template <typename Arg, typename... Args>
void pullValuesHelper(QDataStream& stream, Arg& arg, Args&... args) {
    stream >> arg;
    pullValuesHelper(stream, args...);
}

template <typename... Args>
void pullValues(const QByteArray& data, Args&... args) {
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_12);
    pullValuesHelper(stream, args...);
}

void dispatch(const QByteArray& incomingData, QByteArray& data, QString& command)
{
    QDataStream incoming(incomingData);
    incoming.setVersion(QDataStream::Qt_5_12);
    incoming >> command;
    incoming >> data;
}
}

const char* const DeviceManager::UID_PROPERTY = "__OW_DEVICE_UID__";
const QByteArray DeviceManager::SERVER_NAME = "Objectwheel Device Manager";
const QByteArray DeviceManager::BROADCAST_MESSAGE = "Objectwheel Device Discovery Broadcast";
QBasicTimer DeviceManager::s_broadcastTimer;
QUdpSocket* DeviceManager::s_broadcastSocket = nullptr;
QWebSocketServer* DeviceManager::s_webSocketServer = nullptr;

DeviceManager::DeviceManager(QObject* parent) : QObject(parent)
{
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
        pullValues(data, info);
        client->setProperty(UID_PROPERTY, info.value("deviceUid").toString());
        emit connected(info);
    } else {

    }
}
