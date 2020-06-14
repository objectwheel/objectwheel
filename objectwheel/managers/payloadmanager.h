#ifndef PAYLOADMANAGER_H
#define PAYLOADMANAGER_H

#include <QSslSocket>
#include <QTimer>

class PayloadManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PayloadManager)
    friend class ApplicationCore;

    enum {
        DataTransferTimeout = 8000,
        FrameSizeInBytes = 524288,
    };

    struct Download {
        QByteArray uid;
        QTimer timer;
        QSslSocket* socket;
        qint64 totalBytes;
    };

    struct Upload {
        QByteArray uid;
        QTimer timer;
        QSslSocket* socket;
        QByteArray data;
    };

public:
    static PayloadManager* instance();

    static void scheduleDownload(const QByteArray& uid);
    static QByteArray scheduleUpload(const QByteArray& data);

    static void cancelDownload(const QByteArray& uid);
    static void cancelUpload(const QByteArray& uid);

private slots:
    static void handleEncrypted(Download* download);
    static void handleReadyRead(Download* download);
    static void handleEncrypted(Upload* upload);
    static void handleBytesWritten(Upload* upload, qint64 bytes);

private:
    static void timeoutDownload(const Download* download);
    static void timeoutUpload(const Upload* upload);

private:
    static Download* downloadFromUid(const QByteArray& uid);
    static Upload* uploadFromUid(const QByteArray& uid);

signals:
    void downloadTimedout(const QByteArray& uid);
    void uploadTimedout(const QByteArray& uid);
    void bytesWritten(const QByteArray& uid, qint64 bytes);
    void readyRead(const QByteArray& uid, QIODevice* device, qint64 totalBytes);

private:
    static PayloadManager* s_instance;
    static QVector<Download*> s_downloads;
    static QVector<Upload*> s_uploads;

private:
    explicit PayloadManager(QObject* parent = nullptr);
    ~PayloadManager() override;
};

#endif // PAYLOADMANAGER_H
