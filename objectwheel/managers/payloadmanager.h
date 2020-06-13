#ifndef PAYLOADMANAGER_H
#define PAYLOADMANAGER_H

#include <QSslSocket>
#include <QTimer>
#include <QPointer>

class PayloadManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PayloadManager)
    friend class ApplicationCore;

    enum {
        MaxSimultaneousConnections = 5,
        AuthenticationTimeout = 5000,
        DataTransferTimeout = 8000,
        FrameSizeInBytes = 524288,
    };

    struct Download {
        QByteArray uid;
        QTimer timer;
        QPointer<QSslSocket> socket;
        qint64 totalBytes;
    };

    struct Upload {
        QByteArray uid;
        QTimer timer;
        QPointer<QSslSocket> socket;
        QByteArray data;
    };

public:
    static PayloadManager* instance();

    static bool hasDownload(const QByteArray& uid);
    static bool hasUpload(const QByteArray& uid);

    static void registerDownload(const QByteArray& uid);
    static QByteArray registerUpload(const QByteArray& data);

    static void cancelDownload(const QByteArray& uid);
    static void cancelUpload(const QByteArray& uid);

private slots:
    static void processNewConnection(QSslSocket* socket);
    static void processData(QSslSocket* socket);

private:
    static void handleBytesWritten(const QByteArray& uid, qint64 bytes);
    static void timeoutDownload(const Download* download);
    static void timeoutUpload(const Upload* upload);

private:
    static int simultaneousConnectionCount(const QSslSocket* socket);
    static bool hasDownload(const QSslSocket* socket);
    static bool hasUpload(const QSslSocket* socket);
    static Download* downloadFromUid(const QByteArray& uid);
    static Download* downloadFromSocket(const QSslSocket* socket);
    static Upload* uploadFromUid(const QByteArray& uid);
    static Upload* uploadFromSocket(const QSslSocket* socket);

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
