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

    enum { CONNECTION_TIMEOUT = 5000 };

public:
    enum ServerCommands {
        Invalid,
        MessageTooLarge = 0x2022,
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
        ResponseCloudBuild
    };
    Q_ENUM(ServerCommands)

public:
    static ServerManager* instance();
    static bool isConnected();

    template<typename... Args>
    static qint64 send(ServerCommands command, Args&&... args) {
        if (isConnected()) {
            return instance()->sendBinaryMessage(UtilityFunctions::pushCbor(command, std::forward<Args>(args)...));
        } else {
            qWarning() << "ServerManager::send: Unable to send the data, server is not connected.";
            return -1;
        }
    }

private slots:
    void onError(QAbstractSocket::SocketError);
    void onSslErrors(const QList<QSslError>&);

private:
    void timerEvent(QTimerEvent* event) override;

private:
    static ServerManager* s_instance;
    static QUrl s_host;
    static QBasicTimer s_connectionTimer;

private:
    explicit ServerManager(const QUrl& host, QObject* parent = nullptr);
    ~ServerManager() override;
};

#endif // SERVERMANAGER_H
