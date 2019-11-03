#ifndef COMMANDDISPATCHER_H
#define COMMANDDISPATCHER_H

#include <QObject>
#include <renderercommands.h>

struct RenderInfo;
struct InitInfo;
class RenderSocket;

class CommandDispatcher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandDispatcher)

public:
    explicit CommandDispatcher(RenderSocket* socket, QObject* parent = nullptr);

public slots:
    void scheduleInitializationProgress(int progress);
    void scheduleRenderDone(const QList<RenderInfo>& infos);
    void schedulePreviewDone(const RenderInfo& info);
    void onDataReceived(const RendererCommands& command, const QByteArray& data);

private:
    void send(RenderSocket* socket, RendererCommands command, const QByteArray& data = QByteArray());

signals:
    void init(const InitInfo& initInfo);
    void bindingUpdate(const QString& uid, const QString& bindingName, const QString& expression);
    void propertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    void formCreation(const QString& dir, const QString& module);
    void preview(const QString& url, const QString& module);
    void controlCreation(const QString& dir, const QString& module, const QString& parentUid);
    void refresh(const QString& formUid);
    void parentUpdate(const QString& newDir, const QString& uid, const QString& parentUid);
    void indexUpdate(const QString& uid);
    void idUpdate(const QString& uid, const QString& newId);
    void controlDeletion(const QString& uid);
    void formDeletion(const QString& uid);
    void controlCodeUpdate(const QString& uid, const QString& module);
    void formCodeUpdate(const QString& uid, const QString& module);
    void devicePixelRatioUpdate(qreal devicePixelRatio);

private:
    RenderSocket* m_socket;
};

#endif // COMMANDDISPATCHER_H