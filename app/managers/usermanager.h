#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <planmanager.h>

class UserManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserManager)

    friend class ApplicationCore;

public:
    static UserManager* instance();
    static QString baseDirectory();
    static QStringList userDirs();
    static QStringList users();
    static QString dir(const QString& = email());

    static PlanManager::Plans plan();
    static QString email();
    static QString password();

    static void logout();
    static bool isLoggedIn();
    static void login(const QString& email, const QString& password);

private slots:
    void onLoginFailure();
    void onLoginSuccessful(const QVariantList& userInfo);

signals:
    void loginFailed();
    void loggedIn();
    void loggedOut();
    void aboutToLogout();

private:
    explicit UserManager(QObject* parent = nullptr);
    ~UserManager() override;

private:
    static UserManager* s_instance;
    static PlanManager::Plans s_plan;
    static QString s_email;
    static QString s_password;
    static QString s_emailCache;
    static QString s_passwordCache;
};

#endif // USERMANAGER_H
