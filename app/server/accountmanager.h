#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>

class AccountManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountManager)

    friend class ApplicationCore;

public:
    static AccountManager* instance();

    static void login(const QString& email, const QString& password);
    static void signup(const QString& first, const QString& last, const QString& email,
                       const QString& password, const QString& country, const QString& company,
                       const QString& title, const QString& phone);
    static void resendSignupCode(const QString& email);
    static void completeSignup(const QString& email, const QString& code);
    static void resetPassword(const QString& email);
    static void completePasswordReset(const QString& email, const QString& password,
                                      const QString& code);

signals:
    void loginSuccessful(const QString& plan);
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
    explicit AccountManager(QObject* parent = nullptr);
    ~AccountManager() override;

private:
    static AccountManager* s_instance;
};

#endif // ACCOUNTMANAGER_H