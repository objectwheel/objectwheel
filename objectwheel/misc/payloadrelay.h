#ifndef PAYLOADRELAY_H
#define PAYLOADRELAY_H

#include <QTimer>
#include <QBuffer>

class QWebSocket;
class PayloadRelay final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PayloadRelay)

    enum {
        DEFAULT_TIMEOUT = 8000,     /* msec */
        DEFAULT_CHUNK_SIZE = 102400 /* 100K */
    };

    struct Payload {
        int receivedBytes;
        QString uid;
        QBuffer buffer;
        QWebSocket* socket;
        QTimer timeoutTimer;
    };

public:
    explicit PayloadRelay(int payloadSymbol, int payloadAckSymbol, QObject* parent = nullptr);
    ~PayloadRelay() override;

    int timeout() const;
    void setTimeout(int timeout);

    int uploadChunkSize() const;
    void setUploadChunkSize(int uploadChunkSize);

    bool downloadBuffered() const;
    void setDownloadBuffered(bool downloadBuffered);

    void registerDownload(QWebSocket* socket, const QString& uid);
    QString scheduleUpload(QWebSocket* socket, const QByteArray& data);

public slots:
    void cancelUpload(const QString& uid);
    void cancelDownload(const QString& uid);

private slots:
    void uploadNextAvailableChunk(Payload* payload);
    void downloadNextAvailableChunk(Payload* payload) const;
    void onBinaryMessageReceived(const QByteArray& message);

private:
    void timeoutUpload(Payload* payload);
    void timeoutDownload(Payload* payload);
    void cleanUpload(Payload* payload);
    void cleanDownload(Payload* payload);

    Payload* uploadPayloadFromUid(const QString& uid) const;
    Payload* downloadPayloadFromUid(const QString& uid) const;

signals:
    void uploadTimedout(const QString& uid);
    void downloadTimedout(const QString& uid);
    void uploadFinished(const QString& uid);
    void bytesUploaded(const QString& uid, int bytes);
    void downloadFinished(const QString& uid, const QByteArray& data);
    void bytesDownloaded(const QString& uid, const QByteArray& chunk, int totalBytes);

private:
    const int m_payloadSymbol;
    const int m_payloadAckSymbol;
    int m_timeout;
    int m_uploadChunkSize;
    bool m_downloadBuffered;
    QVector<Payload*> m_uploads;
    QVector<Payload*> m_downloads;
};

#endif // PAYLOADRELAY_H
