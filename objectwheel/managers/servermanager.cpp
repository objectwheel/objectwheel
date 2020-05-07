#include <servermanager.h>
#include <QTimerEvent>

ServerManager* ServerManager::s_instance = nullptr;
QUrl ServerManager::s_host;
QBasicTimer ServerManager::s_pingTimer;
QElapsedTimer ServerManager::s_activityTimer;

ServerManager::ServerManager(const QUrl& host, QObject* parent)
    : QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent)
{
    s_instance = this;
    s_host = host;
    connect(this, qOverload<QAbstractSocket::SocketError>(&ServerManager::error),
            this, &ServerManager::onError);
    connect(this, &ServerManager::sslErrors,
            this, &ServerManager::onSslErrors);
    connect(this, &ServerManager::textFrameReceived,
            this, [] { s_activityTimer.restart(); });
    connect(this, &ServerManager::binaryFrameReceived,
            this, [] { s_activityTimer.restart(); });
    connect(this, &ServerManager::pong,
            this, [] { s_activityTimer.restart(); });
    connect(this, &ServerManager::connected,
            this, [this] { ping(); s_activityTimer.restart(); });
    s_activityTimer.start();
    s_pingTimer.start(PING_INTERVAL, Qt::VeryCoarseTimer, this);
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
    if (event->timerId() == s_pingTimer.timerId()) {
        if (state() == QAbstractSocket::ConnectedState)
            ping();
        if (s_activityTimer.hasExpired(ACTIVITY_THRESHOLD)) {
            if (state() == QAbstractSocket::UnconnectedState) {
                open(s_host);
            } else { // You have 3 secs to establish a successful connection
                abort();
                open(s_host);
            }
        }
    } else {
        QWebSocket::timerEvent(event);
    }
}
