#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QBuffer>
#include <QCborMap>
#include <QFutureWatcher>
#include <fastdownloader.h>

class QDir;
class UpdateManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateManager)

    friend class ApplicationCore;

public:
    static UpdateManager* instance();
    static QString changelog();
    static qint64 downloadSize();
    static int fileCount();
    static bool isUpdateCheckRunning();
    static bool isDownloadRunning();
    static void startUpdateCheck(bool forceLocalScan = true);
    static void download();
    static void cancelDownload();
    static void install();

private:
    static QDir topUpdateDir();
    static QString topUpdateRemotePath();
    static QCborMap handleLocalScan(QFutureInterfaceBase* futureInterface, const QDir& topDir, const QDir& dir);
    static QCborMap handleDownload(QFutureInterfaceBase* futureInterface);
    static void handleDownloaderError();
    static void updateDownloadInfo();

private slots:
    void onServerManagerConnected();
    void onLocalScanFinish();
    void onChecksumsDownloaderResolved();
    void onChangelogDownloaderResolved();
    void onChecksumsDownloaderFinished();
    void onChangelogDownloaderFinished();
    void onChecksumsDownloaderReadyRead(int id);
    void onChangelogDownloaderReadyRead(int id);
    void onDownloadWatcherResultReadyAt(int resultIndex);
    void onDownloadWatcherFinish();

signals:
    void downloadProgress(qint64 totalBytes, qint64 receivedBytes, qreal speed,
                          int fileCount, int fileIndex, const QString& fileName);
    void downloadFinished(bool canceled, const QString& errorString);
    void updateCheckStarted();
    void updateCheckFinished(bool succeed);

private:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

private:
    static UpdateManager* s_instance;
    static bool s_isUpdateCheckRunning;
    static QBuffer s_checksumsBuffer;
    static QBuffer s_changelogBuffer;
    static FastDownloader s_checksumsDownloader;
    static FastDownloader s_changelogDownloader;
    static QCborMap s_localChecksums;
    static QCborMap s_remoteChecksums;
    static QCborMap s_checksumsDiff;
    static QString s_changelog;
    static QDateTime s_lastSuccessfulCheckup;
    static int s_fileCount;
    static qint64 s_downloadSize;
    static QFutureWatcher<QCborMap> s_downloadWatcher;
    static QFutureWatcher<QCborMap> s_localScanWatcher;
};

#endif // UPDATEMANAGER_H
