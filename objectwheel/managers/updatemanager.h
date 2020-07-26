#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QBuffer>
#include <QCborMap>
#include <QFutureWatcher>
#include <fastdownloader.h>

class UpdateManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateManager)

    friend class ApplicationCore;

public:
    static UpdateManager* instance();
    static qint64 fileCount();
    static qint64 downloadSize();
    static QString changelog();
    static bool isUpdateCheckRunning();
    static bool isDownloadRunning();

public:
    static void startUpdateCheck(bool forceLocalScan = true);
    static void download();
    static void cancelDownload();
    static void install();

private slots:
    void onServerManagerConnected();
    void onChangelogDownloaderResolved();
    void onChecksumsDownloaderResolved();
    void onChangelogDownloaderReadyRead(int id);
    void onChecksumsDownloaderReadyRead(int id);
    void onChangelogDownloaderFinished();
    void onChecksumsDownloaderFinished();
    void onLocalScanFinished();
    void onDownloadWatcherResultReadyAt(int resultIndex);
    void onDownloadWatcherFinished();

private:
    static QString localUpdateRootPath();
    static QString remoteUpdateRootPath();
    static void handleInfoDownloaderError();
    static void handleDownloadInfoUpdate();
    static void doLocalScan(QFutureInterfaceBase* futureInterface);
    static void doDownload(QFutureInterfaceBase* futureInterface);

signals:
    void updateCheckStarted();
    void updateCheckFinished(bool succeed);
    void downloadProgress(qreal bytesPerSec, qint64 bytesReceived, qint64 fileIndex, const QString& fileName);
    void downloadFinished(bool canceled, const QString& errorString);

private:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

private:
    static UpdateManager* s_instance;
    static bool s_isUpdateCheckRunning;
    static qint64 s_fileCount;
    static qint64 s_downloadSize;
    static QDateTime s_lastSuccessfulCheckup;
    static QString s_changelog;
    static QCborMap s_localChecksums;
    static QCborMap s_remoteChecksums;
    static QCborMap s_checksumsDiff;
    static QBuffer s_changelogBuffer;
    static QBuffer s_checksumsBuffer;
    static FastDownloader s_changelogDownloader;
    static FastDownloader s_checksumsDownloader;
    static QFutureWatcher<QCborMap> s_localScanWatcher;
    static QFutureWatcher<QVariantMap> s_downloadWatcher;
};

#endif // UPDATEMANAGER_H
