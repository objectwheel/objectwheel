#ifndef CONTROLPREVIEWINGMANAGER_H
#define CONTROLPREVIEWINGMANAGER_H

#include <QObject>

class PreviewerServer;
class CommandDispatcher;
struct PreviewResult;

class ControlPreviewingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlPreviewingManager)

    friend class ApplicationCore;

public:
    static ControlPreviewingManager* instance();

    static void scheduleParentUpdate(const QString& uid, const QString& parentUid, const QString& newUrl){}
    static void scheduleControlDeletion(){}
    static void scheduleFormCreation(const QString& uid){}
    static void scheduleFormDeletion(const QString& uid){}
    static void scheduleIdChange(const QString& uid, const QString& newId){}
    static void scheduleAnchorChange(){}

    static void scheduleRefresh(const QString& parentUid) {}
    static void scheduleControlCreation(const QString& dir, const QString& parentUid);
    static void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    static void scheduleTerminate();
    static void scheduleInit();

private slots:
    void onConnected();
    void onDisconnected();
    void onConnectionTimeout();
    void onPreviewResultsReady(const QList<PreviewResult>& results);
    void onInitializationProgressChange(int progress);

signals:
    void previewDone(const PreviewResult& result);
    void initializationProgressChanged(int progress);

private:
    explicit ControlPreviewingManager(QObject* parent = nullptr);
    ~ControlPreviewingManager();

private:
    static ControlPreviewingManager* s_instance;
    static PreviewerServer* s_previewerServer;
    static QThread* s_serverThread;
    static CommandDispatcher* s_commandDispatcher;

};

#endif // CONTROLPREVIEWINGMANAGER_H