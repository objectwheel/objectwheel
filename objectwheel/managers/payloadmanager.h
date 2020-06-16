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
        qint64 bytesWritten;
        bool isFinished;
    };

    struct Upload {
        QByteArray uid;
        QTimer timer;
        QSslSocket* socket;
        QByteArray data;
        qint64 bytesLeft;
        bool isFinished;
    };

public:
    static PayloadManager* instance();

    static void scheduleDownload(const QByteArray& uid);
    static QByteArray scheduleUpload(const QByteArray& data);

    static void cancelDownload(const QByteArray& uid, bool abort = true);
    static void cancelUpload(const QByteArray& uid, bool abort = true);

private slots:
    static void handleConnected(Download* download);
    static void handleReadyRead(Download* download);
    static void handleConnected(Upload* upload);
    static void handleBytesWritten(Upload* upload, qint64 bytes);

private:
    static void timeoutDownload(Download* download);
    static void timeoutUpload(Upload* upload);

private:
    static Download* downloadFromUid(const QByteArray& uid);
    static Upload* uploadFromUid(const QByteArray& uid);

signals:
    void downloadTimedout(const QByteArray& uid);
    void uploadTimedout(const QByteArray& uid);
    void downloadAborted(const QByteArray& uid);
    void uploadAborted(const QByteArray& uid);
    void bytesWritten(const QByteArray& uid, qint64 bytes, bool isLastFrame);
    void readyRead(const QByteArray& uid, QIODevice* device, qint64 totalBytes, bool isLastFrame);

private:
    static PayloadManager* s_instance;
    static QVector<Download*> s_downloads;
    static QVector<Upload*> s_uploads;

private:
    explicit PayloadManager(QObject* parent = nullptr);
    ~PayloadManager() override;
};

#endif // PAYLOADMANAGER_H
