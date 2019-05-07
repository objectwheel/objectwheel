#ifndef REGISTRATIONAPIMANAGER_H
#define REGISTRATIONAPIMANAGER_H

#include <servermanager.h>
#include <usermanager.h>

class RegistrationApiManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RegistrationApiManager)

    friend class ApplicationCore;

public:
    static RegistrationApiManager* instance();

    static void login(const QString& email, const QString& password);
    static void signup(const QString& first, const QString& last, const QString& email,
                       const QString& password, const QString& country, const QString& company,
                       const QString& title, const QString& phone);
    static void resendSignupCode(const QString& email);
    static void completeSignup(const QString& email, const QString& code);
    static void resetPassword(const QString& email);
    static void completePasswordReset(const QString& email, const QString& password,
                                      const QString& code);

private slots:
    void onDataArrival(ServerManager::ServerCommands command, const QByteArray& data);

signals:
    void loginSuccessful(UserManager::Plans plan);
    void loginFailure();
    void signupSuccessful();
    void signupFailure();
    void resendSignupCodeSuccessful();
    void resendSignupCodeFailure();
    void completeSignupSuccessful();
    void completeSignupFailure();
    void resetPasswordSuccessful();
    void resetPasswordFailure();
    void completePasswordResetSuccessful();
    void completePasswordResetFailure();

private:
    explicit RegistrationApiManager(QObject* parent = nullptr);
    ~RegistrationApiManager() override;

private:
    static RegistrationApiManager* s_instance;
};

#endif // REGISTRATIONAPIMANAGER_H