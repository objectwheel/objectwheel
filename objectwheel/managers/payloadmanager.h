#ifndef PAYLOADMANAGER_H
#define PAYLOADMANAGER_H

#include <QTimer>
#include <QBuffer>

class QWebSocket;
class QTcpServer;

class PayloadManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PayloadManager)

    enum {
        DEFAULT_TIMEOUT = 8000,     /* msec */
        DEFAULT_CHUNK_SIZE = 256000 /* 250K */
    };

    struct Payload {
        QString uid;
        QBuffer buffer;
        QWebSocket* socket;
        QTimer timeoutTimer;
        QVector<QMetaObject::Connection> connections;
    };

public:
    enum InstallationType {
        ServerSetup,
        ClientSetup
    };

public:
    explicit PayloadManager(InstallationType installationType, QObject* parent = nullptr);
    ~PayloadManager() override;

    bool hasUpload(const QString& uid) const;
    bool hasDownload(const QString& uid) const;

    int timeout() const;
    void setTimeout(int timeout);

    int uploadChunkSize() const;
    void setUploadChunkSize(int uploadChunkSize);

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
    int socketCount(QWebSocket* socket) const;
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
    const InstallationType m_installationType;
    const QTcpServer* m_server;
    int m_timeout;
    int m_uploadChunkSize;
    QVector<Payload*> m_uploads;
    QVector<Payload*> m_downloads;

};

#endif // PAYLOADMANAGER_H
