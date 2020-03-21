#ifndef PAYLOADRELAY_H
#define PAYLOADRELAY_H

#include <QTimer>
#include <QBuffer>
#include <QWebSocket>

class QTimer;
class QWebSocket;

class PayloadRelay final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PayloadRelay)

    enum {
        TIMEOUT = 8000,
        CHUNK_SIZE = 102400 /* 100K */
    };

    struct Payload {
        QString uid;
        QBuffer buffer;
        QWebSocket* socket;
        QTimer timeoutTimer;
    };

public:
    explicit PayloadRelay(int payloadSymbol, int payloadAckSymbol, QObject* parent = nullptr);
    ~PayloadRelay() override;

    void download(QWebSocket* socket, const QString& uid);
    QString upload(QWebSocket* socket, const QByteArray& data);

public slots:
    void cancelUpload(const QString& uid);
    void cancelDownload(const QString& uid);

private slots:
    void uploadNextAvailableChunk(Payload* payload);
    void downloadNextAvailableChunk(Payload* payload);
    void onBinaryMessageReceived(const QByteArray& message);

private:
    void cleanUpload(Payload* payload);
    void cleanDownload(Payload* payload);

    Payload* uploadPayloadFromUid(const QString& uid);
    Payload* downloadPayloadFromUid(const QString& uid);

signals:
    void uploadFinished(const QString& uid);
    void bytesUploaded(const QString& uid, int bytes);
    void downloadFinished(const QString& uid, const QByteArray& data);
    void bytesDownloaded(const QString& uid, const QByteArray& chunk, int totalBytes);

private:
    const int m_payloadSymbol;
    const int m_payloadAckSymbol;
    QVector<Payload*> m_uploads;
    QVector<Payload*> m_downloads;

};

#endif // PAYLOADRELAY_H
