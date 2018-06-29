#include <commanddispatcher.h>
#include <previewerserver.h>
#include <previewresult.h>

namespace {

void pushValuesHelper(QByteArray&) {}

template <typename Arg, typename... Args>
void pushValuesHelper(QByteArray& data, const Arg& arg, const Args&... args) {
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << arg;
    pushValuesHelper(data, args...);
}

template <typename... Args>
QByteArray pushValues(const Args&... args) {
    QByteArray data;
    pushValuesHelper(data, args...);
    return data;
}

void pullValuesHelper(QDataStream&) {}

template <typename Arg, typename... Args>
void pullValuesHelper(QDataStream& dataStream, Arg& arg, Args&... args) {
    dataStream >> arg;
    pullValuesHelper(dataStream, args...);
}

template <typename... Args>
void pullValues(const QByteArray& data, Args&... args) {
    QDataStream stream(data);
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

void CommandDispatcher::schedulePreview(const QString& uid)
{
    sendAsync(m_server, PreviewerCommands::Preview, pushValues(uid));
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
        PreviewResult result;
        pullValues(data, result);
        emit previewDone(result);
        break;
    }

    default:
        break;
    }
}
