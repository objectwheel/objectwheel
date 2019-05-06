#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>

class UserManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserManager)

    friend class ApplicationCore;

public:
    static UserManager* instance();

    static bool exists(const QString& user);
    static void setAutoLogin(const QString& password);

    static QString dir(const QString& = user());

    static const QString& user();
    static const QByteArray& key();

    static void logout();
    static bool hasAutoLogin();
    static bool tryAutoLogin();
    static void clearAutoLogin();
    static bool login(const QString& user, const QString& password);

signals:
    void started();
    void aboutToStop();

private:
    explicit UserManager(QObject* parent = nullptr);
    ~UserManager();

private:
    static UserManager* s_instance;
    static QString s_user;
    static QByteArray s_key;
};

#endif // USERMANAGER_H
