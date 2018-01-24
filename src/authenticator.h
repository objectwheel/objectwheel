#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QWebSocket>

class Authenticator : public QWebSocket
{
        Q_OBJECT
        Q_DISABLE_COPY(Authenticator)

    public:
        static Authenticator* instance();
        void init(const QUrl& host);

    public slots:
        bool signup(
            const QString& recaptcha,
            const QString& first,
            const QString& last,
            const QString& email,
            const QString& password,
            const QString& country, // optional
            const QString& company, // optional
            const QString& title,   // optional
            const QString& phone    // optional
        );
        bool forget(const QString& email);
        bool resend(const QString& email);
        bool login(const QString& email, const QString& password);
        bool verify(const QString& email, const QString& code);
        bool reset(const QString& email, const QString& password, const QString& code);

    private slots:
        void onDisconnected();
        void onError(QAbstractSocket::SocketError error);
        void onSslErrors(const QList<QSslError> &errors);
        void onTextMessageReceived(const QString& message);

    private:
        using QObject::connect;
        bool connect(int timeout);
        QString readSync(int timeout);

    private:
        Authenticator();

    private:
        QUrl _host;
        QString _message
;
};

#endif // AUTHENTICATOR_H