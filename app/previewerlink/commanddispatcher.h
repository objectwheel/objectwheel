#ifndef COMMANDDISPATCHER_H
#define COMMANDDISPATCHER_H

#include <QObject>
#include <previewercommands.h>

struct PreviewResult;
class PreviewerServer;

class CommandDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit CommandDispatcher(PreviewerServer* server, QObject* parent = nullptr);

public slots:
    void scheduleInit();
    void scheduleTerminate();
    void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    void scheduleFormCreation(const QString& dir);
    void scheduleControlCreation(const QString& dir, const QString& parentUid);
    void scheduleRefresh(const QString& formUid);
    void scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid);
    void scheduleIdUpdate(const QString& uid, const QString& newId);
    void scheduleControlDeletion(const QString& uid);
    void scheduleFormDeletion(const QString& uid);
    void scheduleControlCodeUpdate(const QString& uid);
    void scheduleFormCodeUpdate(const QString& uid);
    void onDataReceived(const PreviewerCommands& command, const QByteArray& data);

private:
    void sendAsync(PreviewerServer* server, PreviewerCommands command, const QByteArray& data = QByteArray());

signals:
    void initializationProgressChanged(int progress);
    void previewDone(const QList<PreviewResult>& results);

private:
    PreviewerServer* m_server;
};

#endif // COMMANDDISPATCHER_H