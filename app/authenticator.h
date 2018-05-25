#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QWebSocket>

class Authenticator final : public QWebSocket
{
    Q_OBJECT
    Q_DISABLE_COPY(Authenticator)

    friend class BackendManager;

public:
    static Authenticator* instance();
    static QString login(const QString& email, const QString& password);
    static void setHost(const QUrl& host);
    static bool forget(const QString& email);
    static bool resend(const QString& email);
    static bool verify(const QString& email, const QString& code);
    static bool reset(const QString& email, const QString& password, const QString& code);
    static bool signup(const QString& recaptcha, const QString& first, const QString& last,
                       const QString& email, const QString& password, const QString& country,
                       const QString& company, const QString& title, const QString& phone);
private:
    using QObject::connect;
    static QString readSync(int timeout);
    static bool connect(int timeout);

private slots:
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);
    void onTextMessageReceived(const QString& message);

private:
    explicit Authenticator(QObject* parent = nullptr);
    ~Authenticator();

private:
    static Authenticator* s_instance;
    static QUrl s_host;
    static QString s_message;
};

#endif // AUTHENTICATOR_H