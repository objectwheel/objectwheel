#include <commanddispatcher.h>
#include <rendersocket.h>
#include <renderresult.h>
#include <utilityfunctions.h>

using namespace UtilityFunctions;

CommandDispatcher::CommandDispatcher(RenderSocket* socket, QObject *parent) : QObject(parent)
  , m_socket(socket)
{
    connect(m_socket, &RenderSocket::dataArrived, this, &CommandDispatcher::onDataReceived);
}

void CommandDispatcher::send(RenderSocket* socket, RendererCommands command, const QByteArray& data)
{
    QMetaObject::invokeMethod(socket, "send", Q_ARG(RendererCommands, command), Q_ARG(QByteArray, data));
}

void CommandDispatcher::scheduleInitializationProgress(int progress)
{
    send(m_socket, RendererCommands::InitializationProgress, push(progress));
}

void CommandDispatcher::scheduleRenderDone(const QList<RenderInfo>& infos)
{
    send(m_socket, RendererCommands::RenderDone, push(infos));
}

void CommandDispatcher::schedulePreviewDone(const RenderInfo& info)
{
    send(m_socket, RendererCommands::PreviewDone, push(info));
}

void CommandDispatcher::onDataReceived(const RendererCommands& command, const QByteArray& data)
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

    case IndexUpdate: {
        QString uid;
        pull(data, uid);
        emit indexUpdate(uid);
        break;
    }

    case IdUpdate: {
        QString uid, newId;
        pull(data, uid, newId);
        emit idUpdate(uid, newId);
        break;
    }

    case DevicePixelRatioUpdate: {
        qreal dpr;
        pull(data, dpr);
        emit devicePixelRatioUpdate(dpr);
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

    case Preview: {
        QString url;
        pull(data, url);
        emit preview(url);
        break;
    }

    default:
        break;
    }
}
