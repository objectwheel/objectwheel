#ifndef COMMANDDISPATCHER_H
#define COMMANDDISPATCHER_H

#include <QObject>

#include <previewercommands.h>

class PreviewerServer;

class CommandDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit CommandDispatcher(PreviewerServer* server, QObject* parent = nullptr);

public slots:
    void scheduleInit();
    void scheduleTerminate();
    void onDataReceived(const PreviewerCommands& command, const QByteArray& data);

signals:
    void initializationProgressChanged(int progress);

private:
    PreviewerServer* m_server;
};

#endif // COMMANDDISPATCHER_H