#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QWebSocket>
#include <QBasicTimer>
#include <QDataStream>

class ServerManager final : public QWebSocket
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerManager)

    friend class ApplicationCore;

    enum { CONNECTION_TIMEOUT = 2000 };

public:
    enum ServerCommands {
        Login = 0x2023,
        LoginSuccessful,
        LoginFailure,
        Signup,
        SignupSuccessful,
        SignupFailure,
        ResendSignupCode,
        ResendSignupCodeSuccessful,
        ResendSignupCodeFailure,
        CompleteSignup,
        CompleteSignupSuccessful,
        CompleteSignupFailure,
        ResetPassword,
        ResetPasswordSuccessful,
        ResetPasswordFailure,
        CompletePasswordReset,
        CompletePasswordResetSuccessful,
        CompletePasswordResetFailure
    };

public:
    static ServerManager* instance();

    template<typename... Args>
    static void send(ServerCommands command, Args&&... args);

private slots:
    void onConnect();
    void onDisconnect();
    void onError(QAbstractSocket::SocketError);
    void onSslErrors(const QList<QSslError>&);
    void onBinaryMessageReceive(const QByteArray& message);

private:
    void timerEvent(QTimerEvent* event) override;

signals:
    void dataArrived(ServerCommands command, const QByteArray& data);

private:
    static ServerManager* s_instance;
    static QUrl s_host;
    static QBasicTimer s_connectionTimer;

private:
    explicit ServerManager(const QUrl& host, QObject* parent = nullptr);
    ~ServerManager() override;
};

Q_DECLARE_METATYPE(ServerManager::ServerCommands)

inline QDataStream& operator>>(QDataStream& in, ServerManager::ServerCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const ServerManager::ServerCommands& e)
{ return out << int(e); }

#endif // SERVERMANAGER_H
