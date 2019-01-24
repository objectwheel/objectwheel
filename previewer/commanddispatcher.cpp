#include <commanddispatcher.h>
#include <previewersocket.h>
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
    stream.setVersion(QDataStream::Qt_5_12);
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
    stream.setVersion(QDataStream::Qt_5_12);
    pullValuesHelper(stream, args...);
}

void sendAsync(PreviewerSocket* socket, PreviewerCommands command, const QByteArray& data = QByteArray())
{
    QMetaObject::invokeMethod(socket, "send",
                              Q_ARG(PreviewerCommands, command),
                              Q_ARG(QByteArray, data));
}
}

CommandDispatcher::CommandDispatcher(PreviewerSocket* socket, QObject *parent) : QObject(parent)
  , m_socket(socket)
{
    connect(m_socket, &PreviewerSocket::dataArrived, this, &CommandDispatcher::onDataReceived);
}

void CommandDispatcher::scheduleInitializationProgress(int progress)
{
    sendAsync(m_socket, PreviewerCommands::InitializationProgress, pushValues(progress));
}

void CommandDispatcher::schedulePreviewDone(const QList<PreviewResult>& results)
{
    sendAsync(m_socket, PreviewerCommands::PreviewDone, pushValues(results));
}

void CommandDispatcher::onDataReceived(const PreviewerCommands& command, const QByteArray& data)
{
    switch (command) {
    case Terminate:
        emit terminate();
        break;

    case Init:
        emit init();
        break;

    case FormCodeUpdate: {
        QString uid;
        pullValues(data, uid);
        emit formCodeUpdate(uid);
        break;
    }

    case ControlCodeUpdate: {
        QString uid;
        pullValues(data, uid);
        emit controlCodeUpdate(uid);
        break;
    }

    case FormCreation: {
        QString dir;
        pullValues(data, dir);
        emit formCreation(dir);
        break;
    }

    case FormDeletion: {
        QString uid;
        pullValues(data, uid);
        emit formDeletion(uid);
        break;
    }

    case ControlDeletion: {
        QString uid;
        pullValues(data, uid);
        emit controlDeletion(uid);
        break;
    }

    case IdUpdate: {
        QString uid, newId;
        pullValues(data, uid, newId);
        emit idUpdate(uid, newId);
        break;
    }

    case Refresh: {
        QString formUid;
        pullValues(data, formUid);
        emit refresh(formUid);
        break;
    }

    case ParentUpdate: {
        QString newDir, uid, parentUid;
        pullValues(data, newDir, uid, parentUid);
        emit parentUpdate(newDir, uid, parentUid);
        break;
    }

    case PropertyUpdate: {
        QVariant propertyValue;
        QString uid, propertyName;
        pullValues(data, uid, propertyName, propertyValue);
        emit propertyUpdate(uid, propertyName, propertyValue);
        break;
    }

    case ControlCreation: {
        QString dir, parentUid;
        pullValues(data, dir, parentUid);
        emit controlCreation(dir, parentUid);
        break;
    }

    default:
        break;
    }
}