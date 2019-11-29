#ifndef CONTROLRENDERINGMANAGER_H
#define CONTROLRENDERINGMANAGER_H

#include <QObject>

class RenderServer;
class CommandDispatcher;
class QProcess;
class DesignerScene;
struct RenderInfo;

class ControlRenderingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlRenderingManager)

    friend class ApplicationCore;

public:
    static ControlRenderingManager* instance();

    static void init(DesignerScene* designerScene);
    static void scheduleDevicePixelRatioUpdate(const qreal& value);
    static void scheduleControlCodeUpdate(const QString& uid, const QString& module);
    static void scheduleFormCodeUpdate(const QString& uid, const QString& module);
    static void scheduleFormDeletion(const QString& uid);
    static void scheduleFormCreation(const QString& dir, const QString& module);
    static void scheduleControlDeletion(const QString& uid);
    static void schedulePreview(const QString& url, const QString& module);
    static void scheduleControlCreation(const QString& dir, const QString& module, const QString& parentUid);
    static void scheduleIndexUpdate(const QString& uid);
    static void scheduleIdUpdate(const QString& uid, const QString& newId);
    static void scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid);
    static void scheduleBindingUpdate(const QString& uid, const QString& bindingName, const QString& expression);
    static void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    static void scheduleRefresh(const QString& formUid);

    static void start();
    static void terminate();
    static bool isConnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onConnectionTimeout();
    void onRenderInfosReady(const QList<RenderInfo>& infos);

signals:
    void connectedChanged(bool connected);
    void renderDone(const RenderInfo& info);
    void previewDone(const RenderInfo& info);
    void initializationProgressChanged(int progress);

private:
    explicit ControlRenderingManager(QObject* parent = nullptr);
    ~ControlRenderingManager() override;

private:
    static ControlRenderingManager* s_instance;
    static DesignerScene* s_designerScene;
    static RenderServer* s_renderServer;
    static QThread* s_serverThread;
    static CommandDispatcher* s_commandDispatcher;
    static QProcess* s_process;
    static bool s_terminatedKnowingly;
    static bool s_connected;
};

#endif // CONTROLRENDERINGMANAGER_H