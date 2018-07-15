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

    static void scheduleAnchorChange(){}

    static void scheduleControlCodeUpdate(const QString& uid);
    static void scheduleFormCodeUpdate(const QString& uid);

    static void scheduleFormDeletion(const QString& uid);
    static void scheduleFormCreation(const QString& dir);
    static void scheduleControlDeletion(const QString& uid);
    static void scheduleControlCreation(const QString& dir, const QString& parentUid);
    static void scheduleIdUpdate(const QString& uid, const QString& newId);
    static void scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid);
    static void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    static void scheduleRefresh(const QString& formUid);
    static void scheduleTerminate();
    static void scheduleInit();

private slots:
    void onConnected();
    void onDisconnected();
    void onConnectionTimeout();
    void onPreviewResultsReady(const QList<PreviewResult>& results);

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