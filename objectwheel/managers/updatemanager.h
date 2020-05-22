#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QCborMap>
#include <QFutureWatcher>

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

private:
    static QString hostOS();
    static QDir topDir();
    static QCborMap generateCacheForDir(const QDir& dir);

private slots:
    void onConnect();
    void onDisconnect();
    void onLocalScanFinish();
    void onServerResponse(const QByteArray& data);

signals:
    void updateCheckStarted();
    void updateCheckFinished(bool succeed);

private:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

private:
    static UpdateManager* s_instance;
    static bool s_isUpdateCheckRunning;
    static QCborMap s_localMetaInfo;
    static QCborMap s_remoteMetaInfo;
    static QString s_changelog;
    static QFutureWatcher<QCborMap> s_localMetaInfoWatcher;
};

#endif // UPDATEMANAGER_H
