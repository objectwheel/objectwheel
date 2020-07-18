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
    static void scheduleUpdateCheck(bool force = true);
    static bool isUpdateCheckRunning();
    static qint64 downloadSize();
    static void update();
    static void cancelUpdate();

private:
    static QDir topUpdateDir();
    static QString topUpdateRemotePath();
    static QCborMap generateUpdateChecksums(const QDir& topDir, const QDir& dir);
    static int download(QFutureInterfaceBase* futureInterface);

private slots:
    void onConnect();
    void onDisconnect();
    void onLocalScanFinish();
    void onChecksumsDownloaderResolved();
    void onChangelogDownloaderResolved();
    void onChecksumsDownloaderFinished();
    void onChangelogDownloaderFinished();
    void onChecksumsDownloaderReadyRead(int id);
    void onChangelogDownloaderReadyRead(int id);
    void onUpdateCheckFinish(bool succeed);
    void handleDownloaderError();

signals:
    void downloadStarted();
    void downloadProgress(int progress);
    void downloadFinished(bool succeed);
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
    static qint64 s_downloadSize;
    static QFutureWatcher<int> s_downloadWatcher;
    static QFutureWatcher<QCborMap> s_localChecksumsWatcher;
};

#endif // UPDATEMANAGER_H
