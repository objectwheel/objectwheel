#include <servermanager.h>
#include <coreconstants.h>
#include <QTimerEvent>

ServerManager* ServerManager::s_instance = nullptr;
QBasicTimer ServerManager::s_pingTimer;
QElapsedTimer ServerManager::s_activityTimer;

ServerManager::ServerManager(QObject* parent)
    : QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent)
{
    s_instance = this;
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
    QMetaObject::invokeMethod(this, "open", Qt::QueuedConnection, Q_ARG(QUrl, QUrl(CoreConstants::WSS_URL)));
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
                open(QUrl(CoreConstants::WSS_URL));
            } else { // You have 3 secs to establish a successful connection
                abort();
                open(QUrl(CoreConstants::WSS_URL));
            }
        }
    } else {
        QWebSocket::timerEvent(event);
    }
}
