#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QBasicTimer>
#include <QElapsedTimer>
#include <QWebSocket>
#include <utilityfunctions.h>

class ServerManager final : public QWebSocket
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerManager)

    friend class ApplicationCore;

    enum {
        PingInterval = 3000,
        InactivityLimit = 8000
    };

public:
    enum ServerCommands {
        Invalid = 0x201f,
        MessageTooLarge,
        Login,
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
        ResendPasswordResetCode,
        ResendPasswordResetCodeSuccessful,
        ResendPasswordResetCodeFailure,
        CompletePasswordReset,
        CompletePasswordResetSuccessful,
        CompletePasswordResetFailure,
        RequestCloudBuild,
        ResponseCloudBuild,
        AbortCloudBuild,
        Subscribe,
        SubscriptionSuccessful,
        SubscriptionFailure
    };
    Q_ENUM(ServerCommands)

public:
    static ServerManager* instance();
    static bool isConnected();
    static void sleep();
    static void wake();

    template<typename... Args>
    static qint64 send(ServerCommands command, Args&&... args) {
        Q_ASSERT(isConnected());
        return instance()->sendBinaryMessage(UtilityFunctions::pushCbor(command, std::forward<Args>(args)...));
    }

private slots:
    void onError(QAbstractSocket::SocketError);
    void onSslErrors(const QList<QSslError>&);

private:
    void timerEvent(QTimerEvent* event) override;

private:
    explicit ServerManager(QObject* parent = nullptr);
    ~ServerManager() override;

private:
    static ServerManager* s_instance;
    static QBasicTimer s_pingTimer;
    static QElapsedTimer s_inactivityTimer;
};

#endif // SERVERMANAGER_H
