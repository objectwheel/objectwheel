#ifndef COMMANDDISPATCHER_H
#define COMMANDDISPATCHER_H

#include <QObject>
#include <renderercommands.h>

struct RenderResult;
class RenderServer;

class CommandDispatcher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandDispatcher)

public:
    explicit CommandDispatcher(RenderServer* server, QObject* parent = nullptr);

public slots:
    void scheduleInit();
    void scheduleTerminate();
    void scheduleDevicePixelRatioUpdate(qreal devicePixelRatio);
    void schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    void scheduleFormCreation(const QString& dir);
    void scheduleControlCreation(const QString& dir, const QString& parentUid);
    void schedulePreview(const QString& url);
    void scheduleRefresh(const QString& formUid);
    void scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid);
    void scheduleIndexUpdate(const QString& uid);
    void scheduleIdUpdate(const QString& uid, const QString& newId);
    void scheduleControlDeletion(const QString& uid);
    void scheduleFormDeletion(const QString& uid);
    void scheduleControlCodeUpdate(const QString& uid);
    void scheduleFormCodeUpdate(const QString& uid);
    void scheduleGeometryLock(const QString& uid, bool locked);
    void onDataReceived(const RendererCommands& command, const QByteArray& data);

private:
    void send(RenderServer* server, RendererCommands command, const QByteArray& data = QByteArray());

signals:
    void initializationProgressChanged(int progress);
    void renderDone(const QList<RenderResult>& results);
    void previewDone(const RenderResult& result);

private:
    RenderServer* m_server;
};

#endif // COMMANDDISPATCHER_H