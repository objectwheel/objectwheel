#include <commanddispatcher.h>
#include <previewerserver.h>
#include <previewresult.h>

namespace {

void pushValuesHelper(QDataStream&) {}

template <typename Arg, typename... Args>
void pushValuesHelper(QDataStream& stream, const Arg& arg, const Args&... args) {
    stream << arg;
    pushValuesHelper(stream, args...);
}

template <typename... Args>
QByteArray pushValues(const Args&... args) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_11);
    pushValuesHelper(stream, args...);
    return data;
}

void pullValuesHelper(QDataStream&) {}

template <typename Arg, typename... Args>
void pullValuesHelper(QDataStream& stream, Arg& arg, Args&... args) {
    stream >> arg;
    pullValuesHelper(stream, args...);
}

template <typename... Args>
void pullValues(const QByteArray& data, Args&... args) {
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_11);
    pullValuesHelper(stream, args...);
}

void sendAsync(PreviewerServer* server, PreviewerCommands command, const QByteArray& data = QByteArray())
{
    QMetaObject::invokeMethod(server, "send",
                              Q_ARG(PreviewerCommands, command),
                              Q_ARG(QByteArray, data));
}
}

CommandDispatcher::CommandDispatcher(PreviewerServer* server, QObject *parent) : QObject(parent)
  , m_server(server)
{
    connect(m_server, &PreviewerServer::dataArrived, this, &CommandDispatcher::onDataReceived);
}

void CommandDispatcher::scheduleInit()
{
    sendAsync(m_server, PreviewerCommands::Init);
}

void CommandDispatcher::scheduleTerminate()
{
    sendAsync(m_server, PreviewerCommands::Terminate);
}

void CommandDispatcher::schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue)
{
    sendAsync(m_server, PreviewerCommands::PropertyUpdate, pushValues(uid, propertyName, propertyValue));
}

void CommandDispatcher::scheduleControlCreation(const QString& dir, const QString& parentUid)
{
    sendAsync(m_server, PreviewerCommands::ControlCreation, pushValues(dir, parentUid));
}

void CommandDispatcher::scheduleFormCreation(const QString& dir)
{
    sendAsync(m_server, PreviewerCommands::FormCreation, pushValues(dir));
}

void CommandDispatcher::scheduleRefresh(const QString& formUid)
{
    sendAsync(m_server, PreviewerCommands::Refresh, pushValues(formUid));
}

void CommandDispatcher::scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid)
{
    sendAsync(m_server, PreviewerCommands::ParentUpdate, pushValues(newDir, uid, parentUid));
}

void CommandDispatcher::scheduleIdUpdate(const QString& uid, const QString& newId)
{
    sendAsync(m_server, PreviewerCommands::IdUpdate, pushValues(uid, newId));
}

void CommandDispatcher::scheduleControlDeletion(const QString& uid)
{
    sendAsync(m_server, PreviewerCommands::ControlDeletion, pushValues(uid));
}

void CommandDispatcher::scheduleFormDeletion(const QString& uid)
{
    sendAsync(m_server, PreviewerCommands::FormDeletion, pushValues(uid));
}

void CommandDispatcher::scheduleControlCodeUpdate(const QString& uid)
{
    sendAsync(m_server, PreviewerCommands::ControlCodeUpdate, pushValues(uid));
}

void CommandDispatcher::scheduleFormCodeUpdate(const QString& uid)
{
    sendAsync(m_server, PreviewerCommands::FormCodeUpdate, pushValues(uid));
}

void CommandDispatcher::onDataReceived(const PreviewerCommands& command, const QByteArray& data)
{
    switch (command) {
    case InitializationProgress: {
        int progress;
        pullValues(data, progress);
        emit initializationProgressChanged(progress);
        break;
    }

    case PreviewDone: {
        QList<PreviewResult> results;
        pullValues(data, results);
        emit previewDone(results);
        break;
    }

    default:
        break;
    }
}
