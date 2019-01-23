#include <devicemanager.h>
#include <QUdpSocket>
#include <QWebSocketServer>
#include <QTimerEvent>
#include <QUdpSocket>

const QByteArray DeviceManager::WS_SERVER_NAME = "Objectwheel Device Manager";
const QByteArray DeviceManager::BROADCAST_MESSAGE = "Objectwheel Device Discovery Broadcast";

QBasicTimer DeviceManager::s_broadcastTimer;
QUdpSocket* DeviceManager::s_broadcastSocket = nullptr;
QWebSocketServer* DeviceManager::s_webSocketServer = nullptr;

DeviceManager::DeviceManager(QObject* parent) : QObject(parent)
{
    s_broadcastSocket = new QUdpSocket(this);
    s_webSocketServer = new QWebSocketServer(WS_SERVER_NAME, QWebSocketServer::NonSecureMode, this);
}

void DeviceManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_broadcastTimer.timerId())
        s_broadcastSocket->writeDatagram(BROADCAST_MESSAGE, QHostAddress::Broadcast, BROADCAST_PORT);
    else
        QObject::timerEvent(event);
}
