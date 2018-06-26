#include <controlpreviewingmanager.h>
#include <projectmanager.h>
#include <previewerserver.h>
#include <commanddispatcher.h>

#include <QDebug>
#include <QProcess>
#include <QCoreApplication>

namespace { bool g_initScheduled = false; }

ControlPreviewingManager* ControlPreviewingManager::s_instance = nullptr;
PreviewerServer* ControlPreviewingManager::s_previewerServer = nullptr;
CommandDispatcher* ControlPreviewingManager::s_commandDispatcher = nullptr;

ControlPreviewingManager::ControlPreviewingManager(QObject *parent) : QObject(parent)
{
    s_instance = this;
    s_previewerServer = new PreviewerServer(this);
    s_commandDispatcher = new CommandDispatcher(this);

    connect(s_previewerServer, &PreviewerServer::connected,
            this, &ControlPreviewingManager::onConnected);

    connect(s_previewerServer, &PreviewerServer::disconnected,
            this, &ControlPreviewingManager::onDisconnected);
    connect(s_previewerServer, &PreviewerServer::connectionTimeout,
            this, &ControlPreviewingManager::onConnectionTimeout);

    connect(s_previewerServer, &PreviewerServer::dataArrived,
            s_commandDispatcher, &CommandDispatcher::onDataReceived);

//    connect(s_commandDispatcher, &CommandDispatcher::terminate,
//            this, &ApplicationCore::onTerminateCommand);
//    connect(s_commandDispatcher, &CommandDispatcher::init,
//            s_previewer, &Previewer::init);

    s_previewerServer->listen();
}

ControlPreviewingManager::~ControlPreviewingManager()
{
    s_instance = nullptr;
}

ControlPreviewingManager* ControlPreviewingManager::instance()
{
    return s_instance;
}

void ControlPreviewingManager::scheduleInit()
{
    if (g_initScheduled || s_previewerServer->isConnected()) {
        qWarning() << tr("Terminate existing connection first.");
        return;
    }

    QStringList arguments;
    arguments << ProjectManager::dir();
    arguments << s_previewerServer->serverName();

    QProcess process;
    process.setArguments(arguments);
    process.setProgram(QCoreApplication::applicationDirPath() + "/previewer");
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    process.startDetached();

    g_initScheduled = true;
}

void ControlPreviewingManager::onConnected()
{
    if (g_initScheduled) {
        g_initScheduled = false;
        s_previewerServer->send(PreviewerCommands::Init);
    }
}

void ControlPreviewingManager::onDisconnected()
{
    // TODO
}

void ControlPreviewingManager::onConnectionTimeout()
{
    // TODO
}