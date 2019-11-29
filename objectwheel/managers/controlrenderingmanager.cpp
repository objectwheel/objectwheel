#include <controlrenderingmanager.h>
#include <projectmanager.h>
#include <renderserver.h>
#include <commanddispatcher.h>
#include <hashfactory.h>
#include <toolboxcontroller.h>
#include <designerscene.h>
#include <form.h>
#include <utilityfunctions.h>

#include <QThread>
#include <QProcess>
#include <QCoreApplication>
#include <QLocalServer>
#include <QTimer>
#include <QMessageBox>

ControlRenderingManager* ControlRenderingManager::s_instance = nullptr;
DesignerScene* ControlRenderingManager::s_designerScene = nullptr;
RenderServer* ControlRenderingManager::s_renderServer = nullptr;
QThread* ControlRenderingManager::s_serverThread = nullptr;
CommandDispatcher* ControlRenderingManager::s_commandDispatcher = nullptr;
QProcess* ControlRenderingManager::s_process = nullptr;
QTimer* ControlRenderingManager::s_warningMessageTimer = nullptr;
bool ControlRenderingManager::s_terminatedKnowingly = false;
bool ControlRenderingManager::s_connected = false;

ControlRenderingManager::ControlRenderingManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<RendererCommands>("RendererCommands");

    s_instance = this;
    s_renderServer = new RenderServer;
    s_serverThread = new QThread(this);
    s_process = new QProcess(this);
    s_warningMessageTimer = new QTimer(this);

    s_renderServer->moveToThread(s_serverThread);
    s_serverThread->start();

    s_commandDispatcher = new CommandDispatcher(s_renderServer, this);

    s_process->setProgram(QCoreApplication::applicationDirPath() + "/Renderer");
#if defined(QT_DEBUG)
    s_process->setProcessChannelMode(QProcess::ForwardedChannels);
#else
    s_process->setStandardOutputFile(QProcess::nullDevice());
    s_process->setStandardErrorFile(QProcess::nullDevice());
#endif

    s_warningMessageTimer->setInterval(1000);
    s_warningMessageTimer->setSingleShot(true);

    connect(s_warningMessageTimer, &QTimer::timeout,
            this, &ControlRenderingManager::showRenderEngineCrashWarning);
    connect(s_renderServer, &RenderServer::connected,
            this, &ControlRenderingManager::onConnected);

    connect(s_renderServer, &RenderServer::disconnected,
            this, &ControlRenderingManager::onDisconnected);
    connect(s_renderServer, &RenderServer::connectionTimeout,
            this, &ControlRenderingManager::onConnectionTimeout);

    connect(s_commandDispatcher, &CommandDispatcher::initializationProgressChanged,
            this, &ControlRenderingManager::initializationProgressChanged);
    connect(s_commandDispatcher, &CommandDispatcher::renderDone,
            this, &ControlRenderingManager::onRenderInfosReady);
    connect(s_commandDispatcher, &CommandDispatcher::previewDone,
            this, &ControlRenderingManager::previewDone);

#if defined(RENDERER_DEBUG)
    QLocalServer::removeServer("serverName");
    QMetaObject::invokeMethod(s_renderServer, "listen", Q_ARG(QString, "serverName"));
#else
    QMetaObject::invokeMethod(s_renderServer, "listen", Q_ARG(QString, HashFactory::generate()));
#endif
}

ControlRenderingManager::~ControlRenderingManager()
{
    terminate();
    s_serverThread->quit();
    s_serverThread->wait();

    /*!
        NOTE: We don't post any events in order to terminate any connections within s_renderServer
              And we don't post eny events in order to call deleteLater() on s_renderServer. Since
              none of posted events are capable of being delivered while the app is about to quit.
              But that's not a problem, since ControlRenderingManager is a static class hence
              s_renderServer and all active connections will be removed by OS automatically.

    */

    s_instance = nullptr;
}

ControlRenderingManager* ControlRenderingManager::instance()
{
    return s_instance;
}

void ControlRenderingManager::init(DesignerScene* designerScene)
{
    s_designerScene = designerScene;
}

void ControlRenderingManager::scheduleDevicePixelRatioUpdate(const qreal& value)
{
    s_commandDispatcher->scheduleDevicePixelRatioUpdate(value);
    qputenv("RENDERER_DEVICE_PIXEL_RATIO", QByteArray::number(value));
}

void ControlRenderingManager::scheduleControlCodeUpdate(const QString& uid, const QString& module)
{
    s_commandDispatcher->scheduleControlCodeUpdate(uid, module);
}

void ControlRenderingManager::scheduleFormCodeUpdate(const QString& uid, const QString& module)
{
    s_commandDispatcher->scheduleFormCodeUpdate(uid, module);
}

void ControlRenderingManager::scheduleFormCreation(const QString& dir, const QString& module)
{
    s_commandDispatcher->scheduleFormCreation(dir, module);
}

void ControlRenderingManager::scheduleFormDeletion(const QString& uid)
{
    s_commandDispatcher->scheduleFormDeletion(uid);
}

void ControlRenderingManager::scheduleControlDeletion(const QString& uid)
{
    s_commandDispatcher->scheduleControlDeletion(uid);
}

void ControlRenderingManager::schedulePreview(const QString& url, const QString& module)
{
    s_commandDispatcher->schedulePreview(url, module);
}

void ControlRenderingManager::scheduleIndexUpdate(const QString& uid)
{
    s_commandDispatcher->scheduleIndexUpdate(uid);
}

void ControlRenderingManager::scheduleIdUpdate(const QString& uid, const QString& newId)
{
    s_commandDispatcher->scheduleIdUpdate(uid, newId);
}

void ControlRenderingManager::scheduleRefresh(const QString& formUid)
{
    s_commandDispatcher->scheduleRefresh(formUid);
}

void ControlRenderingManager::scheduleParentUpdate(const QString& newDir, const QString& uid, const QString& parentUid)
{
    s_commandDispatcher->scheduleParentUpdate(newDir, uid, parentUid);
}

void ControlRenderingManager::scheduleControlCreation(const QString& dir, const QString& module, const QString& parentUid)
{
    s_commandDispatcher->scheduleControlCreation(dir, module, parentUid);
}

void ControlRenderingManager::scheduleBindingUpdate(const QString& uid, const QString& bindingName, const QString& expression)
{
    s_commandDispatcher->scheduleBindingUpdate(uid, bindingName, expression);
}

void ControlRenderingManager::schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue)
{
    s_commandDispatcher->schedulePropertyUpdate(uid, propertyName, propertyValue);
}

void ControlRenderingManager::start()
{
    QStringList arguments;
    arguments.append(ProjectManager::dir());
    arguments.append(s_renderServer->serverName());
    s_process->setArguments(arguments);
#if !defined(RENDERER_DEBUG)
    s_process->start();
#endif
}

void ControlRenderingManager::terminate()
{
    s_terminatedKnowingly = true;
    s_process->kill();
    s_process->waitForFinished(1000);
}

bool ControlRenderingManager::isConnected()
{
    return s_connected;
}

void ControlRenderingManager::onConnected()
{
    s_terminatedKnowingly = false;
    InitInfo initInfo;
    QList<Form*> forms = s_designerScene->forms();
    std::sort(forms.begin(), forms.end(), [] (const Control* left, const Control* right) {
        return left->index() < right->index();
    });
    for (Form* form : qAsConst(forms)) {
        QVector<QPair<QString, QString>> childrenInfo;
        const QList<Control*>& children = form->childControls();
        for (Control* childControl : children)
            childrenInfo.append(QPair<QString, QString>(childControl->dir(), childControl->module()));
        initInfo.children.insert(form->dir(), childrenInfo);
        initInfo.forms.append(QPair<QString, QString>(form->dir(), form->module()));
    }
    s_commandDispatcher->scheduleInit(initInfo, ToolboxController::toolboxInitInfo());
    s_connected = true;
    emit connectedChanged(s_connected);
}

void ControlRenderingManager::onDisconnected()
{
    s_connected = false;
    if (s_terminatedKnowingly) {
        s_terminatedKnowingly = false;
        emit connectedChanged(s_connected);
        return;
    }
    if (s_process->state() != QProcess::NotRunning)
        terminate();

    // Give others some space to handle disconnection before warning user
    s_warningMessageTimer->start();

    emit connectedChanged(s_connected);
}

void ControlRenderingManager::onConnectionTimeout()
{
    // TODO
    qWarning() << "Connection timeout, in" << __FILE__ << ":" << __LINE__;
}

void ControlRenderingManager::showRenderEngineCrashWarning()
{
    if (ProjectManager::isStarted()) {
        QMessageBox::StandardButton result = UtilityFunctions::showMessage(
                    nullptr, tr("Rendering Engine Crashed"),
                    tr("Would you like to start it over again?"),
                    QMessageBox::Question,
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (result == QMessageBox::Yes)
            start();
    }
}

void ControlRenderingManager::onRenderInfosReady(const QList<RenderInfo>& infos)
{
    for (const RenderInfo& info : infos)
        emit renderDone(info);
}
