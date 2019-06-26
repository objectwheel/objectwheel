#include <commanddispatcher.h>
#include <renderserver.h>
#include <renderresult.h>
#include <utilityfunctions.h>

using namespace UtilityFunctions;

CommandDispatcher::CommandDispatcher(RenderServer* server, QObject *parent) : QObject(parent)
  , m_server(server)
{
    connect(m_server, &RenderServer::dataArrived, this, &CommandDispatcher::onDataReceived);
}

void CommandDispatcher::send(RenderServer* server, RendererCommands command, const QByteArray& data)
{
    QMetaObject::invokeMethod(server, "send", Q_ARG(RendererCommands, command), Q_ARG(QByteArray, data));
}

void CommandDispatcher::scheduleInit()
{
    send(m_server, RendererCommands::Init);
}

void CommandDispatcher::scheduleTerminate()
{
    send(m_server, RendererCommands::Terminate);
}

void CommandDispatcher::scheduleDevicePixelRatioUpdate(qreal devicePixelRatio)
{
    send(m_server, RendererCommands::DevicePixelRatioUpdate, push(devicePixelRatio));
}

void CommandDispatcher::schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue)
{
    send(m_server, RendererCommands::PropertyUpdate, push(uid, propertyName, propertyValue));
}

void CommandDispatcher::scheduleControlCreation(const QString& dir, const QString& parentUid)
{
    send(m_server, RendererCommands::ControlCreation, push(dir, parentUid));
}

void CommandDispatcher::schedulePreview(const QString& url)
{
    send(m_server, RendererCommands::Preview, push(url));
}

void CommandDispatcher::scheduleFormCreation(const QString& dir)
{
    send(m_server, RendererCommands::FormCreation, push(dir));
}

void CommandDispatcher::scheduleRefresh(const QString& formUid)
{
    send(m_server, RendererCommands::Refresh, push(formUid));
}

void CommandDispatcher::scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid)
{
    send(m_server, RendererCommands::ParentUpdate, push(newDir, uid, parentUid));
}

void CommandDispatcher::scheduleIndexUpdate(const QString& uid)
{
    send(m_server, RendererCommands::IndexUpdate, push(uid));
}

void CommandDispatcher::scheduleIdUpdate(const QString& uid, const QString& newId)
{
    send(m_server, RendererCommands::IdUpdate, push(uid, newId));
}

void CommandDispatcher::scheduleControlDeletion(const QString& uid)
{
    send(m_server, RendererCommands::ControlDeletion, push(uid));
}

void CommandDispatcher::scheduleFormDeletion(const QString& uid)
{
    send(m_server, RendererCommands::FormDeletion, push(uid));
}

void CommandDispatcher::scheduleControlCodeUpdate(const QString& uid)
{
    send(m_server, RendererCommands::ControlCodeUpdate, push(uid));
}

void CommandDispatcher::scheduleFormCodeUpdate(const QString& uid)
{
    send(m_server, RendererCommands::FormCodeUpdate, push(uid));
}

void CommandDispatcher::onDataReceived(const RendererCommands& command, const QByteArray& data)
{
    switch (command) {
    case InitializationProgress: {
        int progress;
        pull(data, progress);
        emit initializationProgressChanged(progress);
        break;
    }

    case RenderDone: {
        QList<RenderResult> results;
        pull(data, results);
        emit renderDone(results);
        break;
    }

    case PreviewDone: {
        QImage preview;
        pull(data, preview);
        emit previewDone(preview);
        break;
    }

    default:
        break;
    }
}