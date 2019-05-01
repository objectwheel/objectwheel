#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QWebSocket>
#include <QBasicTimer>
#include <QDataStream>
#include <utilityfunctions.h>

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
    static void send(ServerCommands command, Args&&... args)
    {
        using namespace UtilityFunctions;
        if (instance()->state() != QAbstractSocket::ConnectedState) {
            qWarning() << "ServerManager::send: Unable to send the data, server is not connected.";
            return;
        }
        instance()->sendBinaryMessage(push(command, push(std::forward<Args>(args)...)));
    }

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

QDataStream& operator>> (QDataStream& in, ServerManager::ServerCommands& e);
QDataStream& operator<< (QDataStream& out, ServerManager::ServerCommands e);

#endif // SERVERMANAGER_H
