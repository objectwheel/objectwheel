#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QCborMap>

class UpdateManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateManager)

    friend class ApplicationCore;

public:
    static UpdateManager* instance();
    static QCborMap updateMetaInfo();
    static void scheduleUpdateCheck();

private:
    static QString hostOS();

private slots:
    void onServerResponse(const QByteArray& data);

signals:
    void metaInfoChanged();

private:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

private:
    static UpdateManager* s_instance;
    static bool s_updateCheckScheduled;
    static QCborMap s_updateMetaInfo;
};

#endif // UPDATEMANAGER_H
