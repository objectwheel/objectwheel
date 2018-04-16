#ifndef USERBACKEND_H
#define USERBACKEND_H

#include <QObject>

class UserBackend : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserBackend)

public:
    static UserBackend* instance();

    bool exists(const QString& user);
    bool newUser(const QString& user);
    void setAutoLogin(const QString& password);

    const QString& user() const;
    const QString& token() const;
    const QByteArray& key() const;
    QString dir(const QString& = instance()->user());

public slots:
    void stop();
    bool hasAutoLogin();
    bool tryAutoLogin();
    void clearAutoLogin();
    bool start(const QString& user, const QString& password);

signals:
    void aboutToStop();

private:
    UserBackend() {}

private:
    QString _user;
    QString _token;
    QByteArray _key;
};

#endif // USERBACKEND_H
