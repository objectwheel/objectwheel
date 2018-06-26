#ifndef CONTROLPREVIEWINGMANAGER_H
#define CONTROLPREVIEWINGMANAGER_H

#include <QObject>

class PreviewerServer;

class ControlPreviewingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlPreviewingManager)

public:
    static ControlPreviewingManager* instance();

    static void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    static void scheduleParentUpdate(const QString& uid, const QString& parentUid, const QString& newUrl);
    static void scheduleControlCreation();
    static void scheduleControlDeletion();
    static void scheduleFormCreation(const QString& uid);
    static void scheduleFormDeletion(const QString& uid);
    static void scheduleIdChange(const QString& uid, const QString& newId);
    static void scheduleAnchorChange();
    static void scheduleRender(const QString& uid);
    static void scheduleTerminate();
    static void scheduleInit();

private slots:
    void onConnected();

signals:
    void previewReady();

private:
    explicit ControlPreviewingManager(QObject* parent = nullptr);
    ~ControlPreviewingManager();

private:
    static ControlPreviewingManager* s_instance;
    static PreviewerServer* s_previewerServer;

};

#endif // CONTROLPREVIEWINGMANAGER_H