#ifndef COMMANDDISPATCHER_H
#define COMMANDDISPATCHER_H

#include <QObject>

#include <previewercommands.h>

class PreviewerServer;
struct PreviewResult;

class CommandDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit CommandDispatcher(PreviewerServer* server, QObject* parent = nullptr);

public slots:
    void scheduleInit();
    void scheduleTerminate();
    void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);

    void onDataReceived(const PreviewerCommands& command, const QByteArray& data);

signals:
    void initializationProgressChanged(int progress);
    void previewDone(const QList<PreviewResult>& results);

private:
    PreviewerServer* m_server;
};

#endif // COMMANDDISPATCHER_H