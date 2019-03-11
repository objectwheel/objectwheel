#include <commanddispatcher.h>
#include <previewersocket.h>
#include <previewresult.h>
#include <utilityfunctions.h>

using namespace UtilityFunctions;

CommandDispatcher::CommandDispatcher(PreviewerSocket* socket, QObject *parent) : QObject(parent)
  , m_socket(socket)
{
    connect(m_socket, &PreviewerSocket::dataArrived, this, &CommandDispatcher::onDataReceived);
}

void CommandDispatcher::sendAsync(PreviewerSocket* socket, PreviewerCommands command, const QByteArray& data)
{
    QMetaObject::invokeMethod(socket, "send", Q_ARG(PreviewerCommands, command), Q_ARG(QByteArray, data));
}

void CommandDispatcher::scheduleInitializationProgress(int progress)
{
    sendAsync(m_socket, PreviewerCommands::InitializationProgress, push(progress));
}

void CommandDispatcher::schedulePreviewDone(const QList<PreviewResult>& results)
{
    sendAsync(m_socket, PreviewerCommands::PreviewDone, push(results));
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
        pull(data, uid);
        emit formCodeUpdate(uid);
        break;
    }

    case ControlCodeUpdate: {
        QString uid;
        pull(data, uid);
        emit controlCodeUpdate(uid);
        break;
    }

    case FormCreation: {
        QString dir;
        pull(data, dir);
        emit formCreation(dir);
        break;
    }

    case FormDeletion: {
        QString uid;
        pull(data, uid);
        emit formDeletion(uid);
        break;
    }

    case ControlDeletion: {
        QString uid;
        pull(data, uid);
        emit controlDeletion(uid);
        break;
    }

    case IdUpdate: {
        QString uid, newId;
        pull(data, uid, newId);
        emit idUpdate(uid, newId);
        break;
    }

    case Refresh: {
        QString formUid;
        pull(data, formUid);
        emit refresh(formUid);
        break;
    }

    case ParentUpdate: {
        QString newDir, uid, parentUid;
        pull(data, newDir, uid, parentUid);
        emit parentUpdate(newDir, uid, parentUid);
        break;
    }

    case PropertyUpdate: {
        QVariant propertyValue;
        QString uid, propertyName;
        pull(data, uid, propertyName, propertyValue);
        emit propertyUpdate(uid, propertyName, propertyValue);
        break;
    }

    case ControlCreation: {
        QString dir, parentUid;
        pull(data, dir, parentUid);
        emit controlCreation(dir, parentUid);
        break;
    }

    default:
        break;
    }
}
