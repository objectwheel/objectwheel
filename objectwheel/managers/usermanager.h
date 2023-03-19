#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>

class UserManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserManager)

    friend class ApplicationCore;
    friend class SubscriptionWindow;

public:
    static UserManager* instance();
    static QString baseDirectory();
    static QStringList userDirs();
    static QStringList users();
    static QString dir(const QString& = email());
    static QString email();
    static QString password();
    static qint64 plan();

    static bool isLoggedIn();
    static bool hasLocalData(const QString& email);
    static void logout();
    static void login(const QString& email, const QString& password);
    static void loginOffline(const QString& email, const QString& password);

private slots:
    void onLoginFailure();
    void onLoginSuccessful(qint64 plan, bool online);

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
    static qint64 s_plan;
    static QString s_email;
    static QString s_password;
    static QString s_emailCache;
    static QString s_passwordCache;
};

#endif // USERMANAGER_H
