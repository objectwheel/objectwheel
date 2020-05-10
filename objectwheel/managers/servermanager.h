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
        PING_INTERVAL = 3000,
        ACTIVITY_THRESHOLD = 7000
    };

public:
    enum ServerCommands {
        Invalid = 0x201f,
        MessageTooLarge,
        Payload,
        ResponsePayload,
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
        CompletePasswordReset,
        CompletePasswordResetSuccessful,
        CompletePasswordResetFailure,
        UpdateUserIcon,
        UpdateUserIconSuccessful,
        UpdateUserIconFailure,
        RequestUserIcon,
        ResponseUserIcon,
        RequestCloudBuild,
        ResponseCloudBuild,
        CancelCloudBuild,
        RequestUpdateMetaInfo,
        ResponseUpdateMetaInfo
    };
    Q_ENUM(ServerCommands)

public:
    static ServerManager* instance();
    static bool isConnected();

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
    explicit ServerManager(const QUrl& host, QObject* parent = nullptr);
    ~ServerManager() override;

private:
    static ServerManager* s_instance;
    static QUrl s_host;
    static QBasicTimer s_pingTimer;
    static QElapsedTimer s_activityTimer;
};

#endif // SERVERMANAGER_H
