#ifndef APIAI_H
#define APIAI_H

#include <QObject>
#include <QWebSocket>

class ApiAiPrivate;

class ApiAi : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString token READ token WRITE setToken)
    Q_PROPERTY(QString language READ language WRITE setLanguage)
    Q_PROPERTY(bool state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool error READ error)

private:
    ApiAiPrivate* _d;
    QWebSocket* m_webSocket;
    QString m_token;
    QString m_language;
    bool m_error;

public:
    explicit ApiAi(QObject *parent = 0);
    ~ApiAi();
    const QString& token() const;
    void setToken(const QString& token);
    const QString& language() const;
    void setLanguage(const QString& language);
    bool state() const;
    bool error() const;

public slots:
    void open();
    void send(const QByteArray& data);
    void flush();
    void close();

signals:
    void stateChanged() const;
    void readyResponse(const QString& response) const;

private slots:
    void handleError(QAbstractSocket::SocketError error);
    void handleStateChanges(QAbstractSocket::SocketState state);

public:
    static QString generateRandomId();
};

#endif // APIAI_H
