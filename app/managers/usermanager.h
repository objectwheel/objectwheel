#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>

class UserManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserManager)

    friend class ApplicationCore;

public:
    enum Plans : quint32 {
        Free,
        Pro = 0x23b,
        Enterprise = 0x5ad
    };

public:
    static UserManager* instance();
    static QStringList users();
    static QString email();
    static QString password();
    static QString dir(const QString& = email());

    static void logout();
    static void clearAutoLogin();
    static bool hasAutoLogin();
    static bool exists(const QString& email);
    static bool login(const QString& email = QString(), const QString& password = QString());

signals:
    void loggedIn();
    void aboutToLogout();

private:
    explicit UserManager(QObject* parent = nullptr);
    ~UserManager();

private:
    static UserManager* s_instance;
    static Plans s_plan;
    static QString s_email;
    static QString s_password;
};

#endif // USERMANAGER_H
