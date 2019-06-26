#ifndef CONTROLRENDERINGMANAGER_H
#define CONTROLRENDERINGMANAGER_H

#include <QObject>

class RenderServer;
class CommandDispatcher;
struct RenderResult;

class ControlRenderingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlRenderingManager)

    friend class ApplicationCore;

public:
    static ControlRenderingManager* instance();

    static void scheduleAnchorChange(){}

    static void scheduleControlCodeUpdate(const QString& uid);
    static void scheduleFormCodeUpdate(const QString& uid);
    static void scheduleFormDeletion(const QString& uid);
    static void scheduleFormCreation(const QString& dir);
    static void scheduleControlDeletion(const QString& uid);
    static void schedulePreview(const QString& url);
    static void scheduleControlCreation(const QString& dir, const QString& parentUid);
    static void scheduleIndexUpdate(const QString& uid);
    static void scheduleIdUpdate(const QString& uid, const QString& newId);
    static void scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid);
    static void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    static void scheduleRefresh(const QString& formUid);
    static void scheduleTerminate();
    static void scheduleInit();

    static qreal devicePixelRatio();
    static void setDevicePixelRatio(const qreal& value);

private slots:
    void onConnected();
    void onDisconnected();
    void onConnectionTimeout();
    void onRenderResultsReady(const QList<RenderResult>& results);

signals:
    void renderDone(const RenderResult& result);
    void previewDone(const QImage& preview);
    void initializationProgressChanged(int progress);

private:
    explicit ControlRenderingManager(QObject* parent = nullptr);
    ~ControlRenderingManager() override;

private:
    static ControlRenderingManager* s_instance;
    static RenderServer* s_renderServer;
    static QThread* s_serverThread;
    static CommandDispatcher* s_commandDispatcher;
    static qreal s_devicePixelRatio;
};

#endif // CONTROLRENDERINGMANAGER_H