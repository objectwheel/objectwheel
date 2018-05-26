#ifndef USERBACKEND_H
#define USERBACKEND_H

#include <QObject>

class UserBackend final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserBackend)

    friend class BackendManager;

public:
    static UserBackend* instance();

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
    explicit UserBackend(QObject* parent = nullptr);
    ~UserBackend();

private:
    static UserBackend* s_instance;
    static QString s_user;
    static QString s_token;
    static QByteArray s_key;
};

#endif // USERBACKEND_H
