#include <devicemanager.h>
#include <QUdpSocket>
#include <QWebSocketServer>
#include <QTimerEvent>

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

    connect(s_webSocketServer, &QWebSocketServer::acceptError, this, [=] {
        qDebug() << "acceptError";
    });
    connect(s_webSocketServer, &QWebSocketServer::closed, this, [=] {
        qDebug() << "closed";
    });
    connect(s_webSocketServer, &QWebSocketServer::newConnection, this, [=] {
        qDebug() << "newConnection";
    });
    connect(s_webSocketServer, &QWebSocketServer::peerVerifyError, this, [=] {
        qDebug() << "peerVerifyError";
    });
    connect(s_webSocketServer, &QWebSocketServer::serverError, this, [=] {
        qDebug() << "serverError";
    });
    connect(s_webSocketServer, &QWebSocketServer::sslErrors, this, [=] {
        qDebug() << "sslErrors";
    });


}

void DeviceManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_broadcastTimer.timerId())
        s_broadcastSocket->writeDatagram(BROADCAST_MESSAGE, QHostAddress::Broadcast, BROADCAST_PORT);
    else
        QObject::timerEvent(event);
}
