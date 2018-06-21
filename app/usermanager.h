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
    static bool newUser(const QString& user);
    static void setAutoLogin(const QString& password);

    static QString dir(const QString& = user());

    static const QString& user();
    static const QString& token();
    static const QByteArray& key();

    static void stop();
    static bool hasAutoLogin();
    static bool tryAutoLogin();
    static void clearAutoLogin();
    static bool start(const QString& user, const QString& password);

signals:
    void aboutToStop();

private:
    explicit UserManager(QObject* parent = nullptr);
    ~UserManager();

private:
    static UserManager* s_instance;
    static QString s_user;
    static QString s_token;
    static QByteArray s_key;
};

#endif // USERMANAGER_H
