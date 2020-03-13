#include <servermanager.h>
#include <QTimerEvent>

ServerManager* ServerManager::s_instance = nullptr;
QUrl ServerManager::s_host;
QBasicTimer ServerManager::s_connectionRetryTimer;
QBasicTimer ServerManager::s_connectionDropTimer;

ServerManager::ServerManager(const QUrl& host, QObject* parent)
    : QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent)
{
    s_instance = this;
    s_host = host;
    connect(this, qOverload<QAbstractSocket::SocketError>(&ServerManager::error),
            this, &ServerManager::onError);
    connect(this, &ServerManager::sslErrors,
            this, &ServerManager::onSslErrors);
    connect(this, &ServerManager::pong,
            this, &ServerManager::resetConnectionDropTimer);
    connect(this, &ServerManager::bytesWritten,
            this, &ServerManager::resetConnectionDropTimer, Qt::QueuedConnection);
    connect(this, &ServerManager::binaryMessageReceived,
            this, &ServerManager::resetConnectionDropTimer, Qt::QueuedConnection);
    connect(this, &ServerManager::stateChanged,
            this, [=] (QAbstractSocket::SocketState state) {
        if (state == QAbstractSocket::UnconnectedState)
            s_connectionDropTimer.stop();
        else
            resetConnectionDropTimer();
    });
    s_connectionRetryTimer.start(CONNECTION_RETRY_TIMEOUT, Qt::VeryCoarseTimer, instance());
    QMetaObject::invokeMethod(this, "open", Qt::QueuedConnection, Q_ARG(QUrl, s_host));
}

ServerManager::~ServerManager()
{
    s_instance = nullptr;
}

ServerManager* ServerManager::instance()
{
    return s_instance;
}

bool ServerManager::isConnected()
{
    return instance()->state() == QAbstractSocket::ConnectedState;
}

void ServerManager::resetConnectionDropTimer()
{
    s_connectionDropTimer.start(CONNECTION_DROP_TIMEOUT, Qt::VeryCoarseTimer, instance());
}

void ServerManager::onError(QAbstractSocket::SocketError error)
{
    if (error != QAbstractSocket::SocketTimeoutError)
        qWarning() << "ServerManager Socket Error: " << errorString();
}

void ServerManager::onSslErrors(const QList<QSslError>& errors)
{
    for (const QSslError& error : errors)
        qWarning() << "ServerManager SSL Error: " << error.errorString();
}

void ServerManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_connectionRetryTimer.timerId()) {
        if (state() == QAbstractSocket::UnconnectedState)
            open(s_host);
        else if (state() == QAbstractSocket::ConnectedState)
            ping();
    } else if (event->timerId() == s_connectionDropTimer.timerId()) {
        if (state() != QAbstractSocket::UnconnectedState)
            close();
    } else {
        QWebSocket::timerEvent(event);
    }
}
