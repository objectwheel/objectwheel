#ifndef AISPEAK_H
#define AISPEAK_H

#include <QObject>
#include <QNetworkReply>

class AiSpeakPrivate;

class AiSpeak : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AiSpeak)
    Q_PROPERTY(QString token READ token WRITE setToken)
    Q_PROPERTY(QString language READ language WRITE setLanguage)

private:
    AiSpeakPrivate* _d;
    QString m_token;
    QString m_language;

public:
    explicit AiSpeak(QObject *parent = 0);
    ~AiSpeak() override;
    const QString& token() const;
    void setToken(const QString& token);
    const QString& language() const;
    void setLanguage(const QString& language);

public slots:
    void speak(const QString& text);

signals:
    void errorOccurred() const;
    void readySpeak(const QByteArray& data) const;

private slots:
    void handleResponse();
    void handleError(QNetworkReply::NetworkError error);

public:
    static QString generateRandomId();
};

#endif // AISPEAK_H
