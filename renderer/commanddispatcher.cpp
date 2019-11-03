#include <commanddispatcher.h>
#include <rendersocket.h>
#include <renderinfo.h>
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
    case Init: {
        InitInfo initInfo;
        pull(data, initInfo);
        emit init(initInfo);
    } break;

    case FormCodeUpdate: {
        QString uid, module;
        pull(data, uid, module);
        emit formCodeUpdate(uid, module);
    } break;

    case ControlCodeUpdate: {
        QString uid, module;
        pull(data, uid, module);
        emit controlCodeUpdate(uid, module);
    } break;

    case FormCreation: {
        QString dir, module;
        pull(data, dir, module);
        emit formCreation(dir, module);
    } break;

    case FormDeletion: {
        QString uid;
        pull(data, uid);
        emit formDeletion(uid);
    } break;

    case ControlDeletion: {
        QString uid;
        pull(data, uid);
        emit controlDeletion(uid);
    } break;

    case IndexUpdate: {
        QString uid;
        pull(data, uid);
        emit indexUpdate(uid);
    } break;

    case IdUpdate: {
        QString uid, newId;
        pull(data, uid, newId);
        emit idUpdate(uid, newId);
    } break;

    case DevicePixelRatioUpdate: {
        qreal dpr;
        pull(data, dpr);
        emit devicePixelRatioUpdate(dpr);
    } break;

    case Refresh: {
        QString formUid;
        pull(data, formUid);
        emit refresh(formUid);
    } break;

    case ParentUpdate: {
        QString newDir, uid, parentUid;
        pull(data, newDir, uid, parentUid);
        emit parentUpdate(newDir, uid, parentUid);
    } break;

    case BindingUpdate: {
        QString uid, bindingName, expression;
        pull(data, uid, bindingName, expression);
        emit bindingUpdate(uid, bindingName, expression);
    } break;

    case PropertyUpdate: {
        QVariant propertyValue;
        QString uid, propertyName;
        pull(data, uid, propertyName, propertyValue);
        emit propertyUpdate(uid, propertyName, propertyValue);
    } break;

    case ControlCreation: {
        QString dir, module, parentUid;
        pull(data, dir, module, parentUid);
        emit controlCreation(dir, module, parentUid);
    } break;

    case Preview: {
        QString url, module;
        pull(data, url, module);
        emit preview(url, module);
    } break;

    default:
        break;
    }
}
