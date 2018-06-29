#ifndef CONTROLPREVIEWINGMANAGER_H
#define CONTROLPREVIEWINGMANAGER_H

#include <QObject>

class PreviewerServer;
class CommandDispatcher;

class ControlPreviewingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlPreviewingManager)

    friend class ApplicationCore;

public:
    static ControlPreviewingManager* instance();

    static void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue){}
    static void scheduleParentUpdate(const QString& uid, const QString& parentUid, const QString& newUrl){}
    static void scheduleControlCreation(){}
    static void scheduleControlDeletion(){}
    static void scheduleFormCreation(const QString& uid){}
    static void scheduleFormDeletion(const QString& uid){}
    static void scheduleIdChange(const QString& uid, const QString& newId){}
    static void scheduleAnchorChange(){}
    static void scheduleRender(const QString& uid){}
    static void scheduleTerminate();
    static void scheduleInit();

private slots:
    void onConnected();
    void onDisconnected();
    void onConnectionTimeout();

signals:
    void previewReady();
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