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
    static QCborMap updateMetaInfo();
    static bool isUpdateCheckInProgress();
    static void scheduleUpdateCheck();

private:
    static QString hostOS();
    static QDir topDir();
    static QCborMap generateCacheForDir(const QDir& dir);

private slots:
    void onLocalScanFinished();
    void onServerResponse(const QByteArray& data);

signals:
    void updateCheckFinished();

private:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

private:
    static UpdateManager* s_instance;
    static bool s_updateCheckInProgress;
    static QCborMap s_localMetaInfo;
    static QCborMap s_updateMetaInfo;
    static QFutureWatcher<QCborMap> s_localMetaInfoWatcher;
};

#endif // UPDATEMANAGER_H
