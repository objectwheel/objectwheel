#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include <servermanager.h>

class AccountManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountManager)

    friend class ApplicationCore;

public:
    enum Plans {
        Free,
        Pro = 0x23b,
        Enterprise = 0x5ad
    };

public:
    static AccountManager* instance();
    static Plans plan();

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
    void loginSuccessful(const Plans& plan);
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
    static Plans s_plan;
};

#endif // ACCOUNTMANAGER_H