#include <commanddispatcher.h>
#include <previewerserver.h>
#include <previewresult.h>
#include <utilityfunctions.h>

using namespace UtilityFunctions;

CommandDispatcher::CommandDispatcher(PreviewerServer* server, QObject *parent) : QObject(parent)
  , m_server(server)
{
    connect(m_server, &PreviewerServer::dataArrived, this, &CommandDispatcher::onDataReceived);
}

void CommandDispatcher::send(PreviewerServer* server, PreviewerCommands command, const QByteArray& data)
{
    QMetaObject::invokeMethod(server, "send", Q_ARG(PreviewerCommands, command), Q_ARG(QByteArray, data));
}

void CommandDispatcher::scheduleInit()
{
    send(m_server, PreviewerCommands::Init);
}

void CommandDispatcher::scheduleTerminate()
{
    send(m_server, PreviewerCommands::Terminate);
}

void CommandDispatcher::scheduleDevicePixelRatioUpdate(qreal devicePixelRatio)
{
    send(m_server, PreviewerCommands::DevicePixelRatioUpdate, push(devicePixelRatio));
}

void CommandDispatcher::schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue)
{
    send(m_server, PreviewerCommands::PropertyUpdate, push(uid, propertyName, propertyValue));
}

void CommandDispatcher::scheduleControlCreation(const QString& dir, const QString& parentUid)
{
    send(m_server, PreviewerCommands::ControlCreation, push(dir, parentUid));
}

void CommandDispatcher::scheduleIndividualPreview(const QString& url)
{
    send(m_server, PreviewerCommands::IndividualPreview, push(url));
}

void CommandDispatcher::scheduleFormCreation(const QString& dir)
{
    send(m_server, PreviewerCommands::FormCreation, push(dir));
}

void CommandDispatcher::scheduleRefresh(const QString& formUid)
{
    send(m_server, PreviewerCommands::Refresh, push(formUid));
}

void CommandDispatcher::scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid)
{
    send(m_server, PreviewerCommands::ParentUpdate, push(newDir, uid, parentUid));
}

void CommandDispatcher::scheduleIndexUpdate(const QString& uid)
{
    send(m_server, PreviewerCommands::IndexUpdate, push(uid));
}

void CommandDispatcher::scheduleIdUpdate(const QString& uid, const QString& newId)
{
    send(m_server, PreviewerCommands::IdUpdate, push(uid, newId));
}

void CommandDispatcher::scheduleControlDeletion(const QString& uid)
{
    send(m_server, PreviewerCommands::ControlDeletion, push(uid));
}

void CommandDispatcher::scheduleFormDeletion(const QString& uid)
{
    send(m_server, PreviewerCommands::FormDeletion, push(uid));
}

void CommandDispatcher::scheduleControlCodeUpdate(const QString& uid)
{
    send(m_server, PreviewerCommands::ControlCodeUpdate, push(uid));
}

void CommandDispatcher::scheduleFormCodeUpdate(const QString& uid)
{
    send(m_server, PreviewerCommands::FormCodeUpdate, push(uid));
}

void CommandDispatcher::onDataReceived(const PreviewerCommands& command, const QByteArray& data)
{
    switch (command) {
    case InitializationProgress: {
        int progress;
        pull(data, progress);
        emit initializationProgressChanged(progress);
        break;
    }

    case PreviewDone: {
        QList<PreviewResult> results;
        pull(data, results);
        emit previewDone(results);
        break;
    }

    case IndividualPreviewDone: {
        QImage preview;
        pull(data, preview);
        emit individualPreviewDone(preview);
        break;
    }

    default:
        break;
    }
}
