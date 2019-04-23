#include <servermanager.h>
#include <utilityfunctions.h>
#include <QTimerEvent>

ServerManager* ServerManager::s_instance = nullptr;
QUrl ServerManager::s_host;
QBasicTimer ServerManager::s_connectionTimer;

ServerManager::ServerManager(const QUrl& host, QObject* parent) : QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent)
{
    s_instance = this;
    s_host = host;

    connect(this, &ServerManager::connected,
            this, &ServerManager::onConnect);
    connect(this, &ServerManager::disconnected,
            this, &ServerManager::onDisconnect);
    connect(this, qOverload<QAbstractSocket::SocketError>(&ServerManager::error),
            this, &ServerManager::onError);
    connect(this, &ServerManager::sslErrors,
            this, &ServerManager::onSslErrors);
    connect(this, &ServerManager::binaryMessageReceived,
            this, &ServerManager::onBinaryMessageReceive);

    s_connectionTimer.start(CONNECTION_TIMEOUT, Qt::VeryCoarseTimer, this);
}

ServerManager::~ServerManager()
{
    s_instance = nullptr;
}

template<typename... Args>
void ServerManager::send(ServerManager::ServerCommands command, Args&&... args)
{
    using namespace UtilityFunctions;
    if (instance()->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "ServerManager::send: Unable to send the data, server is not connected.";
        return;
    }
    instance()->sendBinaryMessage(push(command, push(std::forward<Args>(args)...)));
}

void ServerManager::onBinaryMessageReceive(const QByteArray& message)
{
    QByteArray data;
    ServerCommands command;
    UtilityFunctions::pull(message, command, data);
    emit dataArrived(command, data);
}

ServerManager* ServerManager::instance()
{
    return s_instance;
}

void ServerManager::onConnect()
{
    s_connectionTimer.stop();
}

void ServerManager::onDisconnect()
{
    s_connectionTimer.start(CONNECTION_TIMEOUT, Qt::VeryCoarseTimer, this);
}

void ServerManager::onError(QAbstractSocket::SocketError)
{
    qWarning() << "ServerManager Socket Error: " << errorString();
}

void ServerManager::onSslErrors(const QList<QSslError>& errors)
{
    for (const QSslError& error : errors)
        qWarning() << "ServerManager SSL Error: " << error.errorString();
}

void ServerManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_connectionTimer.timerId()
            && state() == QAbstractSocket::UnconnectedState) {
        open(s_host);
    } else {
        QWebSocket::timerEvent(event);
    }
}