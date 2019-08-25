#include <controlrenderingmanager.h>
#include <projectmanager.h>
#include <renderserver.h>
#include <commanddispatcher.h>
#include <hashfactory.h>
#include <renderinfo.h>
#include <utilityfunctions.h>

#include <QThread>
#include <QProcess>
#include <QCoreApplication>
#include <QLocalServer>
#include <QMessageBox>

namespace { bool g_initScheduled = false; }

ControlRenderingManager* ControlRenderingManager::s_instance = nullptr;
RenderServer* ControlRenderingManager::s_renderServer = nullptr;
QThread* ControlRenderingManager::s_serverThread = nullptr;
CommandDispatcher* ControlRenderingManager::s_commandDispatcher = nullptr;

ControlRenderingManager::ControlRenderingManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<RendererCommands>("RendererCommands");

    s_instance = this;
    s_renderServer = new RenderServer;
    s_serverThread = new QThread(this);

    s_renderServer->moveToThread(s_serverThread);
    s_serverThread->start();

    s_commandDispatcher = new CommandDispatcher(s_renderServer, this);

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

void ControlRenderingManager::scheduleDevicePixelRatioUpdate(const qreal& value)
{
    s_commandDispatcher->scheduleDevicePixelRatioUpdate(value);
    qputenv("RENDERER_DEVICE_PIXEL_RATIO", QByteArray::number(value));
}

void ControlRenderingManager::scheduleControlCodeUpdate(const QString& uid)
{
    s_commandDispatcher->scheduleControlCodeUpdate(uid);
}

void ControlRenderingManager::scheduleFormCodeUpdate(const QString& uid)
{
    s_commandDispatcher->scheduleFormCodeUpdate(uid);
}

void ControlRenderingManager::scheduleFormCreation(const QString& dir)
{
    s_commandDispatcher->scheduleFormCreation(dir);
}

void ControlRenderingManager::scheduleFormDeletion(const QString& uid)
{
    s_commandDispatcher->scheduleFormDeletion(uid);
}

void ControlRenderingManager::scheduleControlDeletion(const QString& uid)
{
    s_commandDispatcher->scheduleControlDeletion(uid);
}

void ControlRenderingManager::schedulePreview(const QString& url)
{
    s_commandDispatcher->schedulePreview(url);
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

void ControlRenderingManager::scheduleControlCreation(const QString& dir, const QString& parentUid)
{
    s_commandDispatcher->scheduleControlCreation(dir, parentUid);
}

void ControlRenderingManager::scheduleBindingUpdate(const QString& uid, const QString& bindingName, const QString& expression)
{
    s_commandDispatcher->scheduleBindingUpdate(uid, bindingName, expression);
}

void ControlRenderingManager::schedulePropertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue)
{
    s_commandDispatcher->schedulePropertyUpdate(uid, propertyName, propertyValue);
}

void ControlRenderingManager::scheduleInit()
{
    Q_ASSERT_X(!g_initScheduled, "scheduleInit", "Already scheduled");

    s_serverThread->wait(100); // may scheduleTerminate happens after scheduleInit is called from ProjectManager

    if (s_renderServer->isConnected()) {
        s_commandDispatcher->scheduleInit();
    } else {
        QStringList arguments;
        arguments << ProjectManager::dir();
        arguments << s_renderServer->serverName();

        QProcess process;
        process.setArguments(arguments);
        process.setProgram(QCoreApplication::applicationDirPath() + "/renderer");

#if !defined(QT_DEBUG)
        process.setStandardOutputFile(QProcess::nullDevice());
        process.setStandardErrorFile(QProcess::nullDevice());
#endif

#if !defined(RENDERER_DEBUG)
        process.startDetached();
#endif

        g_initScheduled = true;
    }
}

void ControlRenderingManager::scheduleTerminate()
{
    s_commandDispatcher->scheduleTerminate();
    QMetaObject::invokeMethod(s_renderServer, "abort");
}

void ControlRenderingManager::onConnected()
{
    if (g_initScheduled)
        s_commandDispatcher->scheduleInit();
}

void ControlRenderingManager::onDisconnected()
{
    if (ProjectManager::isStarted()) {
        QMessageBox::StandardButton result = UtilityFunctions::showMessage(
                    nullptr, tr("Rendering Engine Crashed"),
                    tr("Would you like to start it over again?"),
                    QMessageBox::Question,
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (result & QMessageBox::Yes)
            scheduleInit();
    }
}

void ControlRenderingManager::onConnectionTimeout()
{
    // TODO

    qWarning() << "Connection timeout, in" << __FILE__ << ":" << __LINE__;
}

void ControlRenderingManager::onRenderInfosReady(const QList<RenderInfo>& infos)
{
    for (const RenderInfo& info : infos)
        emit renderDone(info);

    if (g_initScheduled)
        g_initScheduled = false;
}
