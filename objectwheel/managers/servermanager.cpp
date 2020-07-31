#include <servermanager.h>
#include <appconstants.h>
#include <QTimerEvent>

ServerManager* ServerManager::s_instance = nullptr;
QBasicTimer ServerManager::s_pingTimer;
QElapsedTimer ServerManager::s_inactivityTimer;

ServerManager::ServerManager(QObject* parent) : QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent)
{
    s_instance = this;
    connect(this, qOverload<QAbstractSocket::SocketError>(&ServerManager::error),
            this, &ServerManager::onError);
    connect(this, &ServerManager::sslErrors,
            this, &ServerManager::onSslErrors);
    connect(this, &ServerManager::textFrameReceived,
            this, [] { s_inactivityTimer.restart(); });
    connect(this, &ServerManager::binaryFrameReceived,
            this, [] { s_inactivityTimer.restart(); });
    connect(this, &ServerManager::pong,
            this, [] { s_inactivityTimer.restart(); });
    connect(this, &ServerManager::connected,
            this, [this] { ping(); s_inactivityTimer.restart(); });
    wake();
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

void ServerManager::sleep()
{
    s_pingTimer.stop();
    s_inactivityTimer.invalidate();
    if (s_instance->state() != QAbstractSocket::UnconnectedState)
        s_instance->abort();
}

void ServerManager::wake()
{
    s_inactivityTimer.start();
    s_pingTimer.start(PingInterval, Qt::VeryCoarseTimer, s_instance);
    QMetaObject::invokeMethod(s_instance, "open", Qt::QueuedConnection, Q_ARG(QUrl, QUrl(AppConstants::WSS_URL)));
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
        if (s_inactivityTimer.hasExpired(InactivityLimit)) {
            if (state() != QAbstractSocket::UnconnectedState)
                abort();
            open(QUrl(AppConstants::WSS_URL));
        }
    } else {
        QWebSocket::timerEvent(event);
    }
}
