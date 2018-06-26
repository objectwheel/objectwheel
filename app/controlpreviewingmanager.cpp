#include <controlpreviewingmanager.h>
#include <projectmanager.h>
#include <previewerserver.h>
#include <commanddispatcher.h>
#include <hashfactory.h>

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

#if defined(PREVIEWER_DEBUG)
    s_previewerServer->removeServer("serverName");
    s_previewerServer->listen("serverName");
#else
    s_previewerServer->listen(HashFactory::generate());
#endif
}

ControlPreviewingManager::~ControlPreviewingManager()
{
    s_previewerServer->send(PreviewerCommands::Terminate);
    s_instance = nullptr;
}

ControlPreviewingManager* ControlPreviewingManager::instance()
{
    return s_instance;
}

void ControlPreviewingManager::scheduleInit()
{
    QStringList arguments;
    arguments << ProjectManager::dir();
    arguments << s_previewerServer->serverName();

    QProcess process;
    process.setArguments(arguments);
    process.setProgram(QCoreApplication::applicationDirPath() + "/previewer");
#if !defined(QT_DEBUG)
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
#endif
    process.startDetached();

    g_initScheduled = true;
}

void ControlPreviewingManager::terminate()
{
    s_previewerServer->send(Terminate);
    s_previewerServer->abort();
}

void ControlPreviewingManager::onConnected()
{
    if (g_initScheduled) {
        g_initScheduled = false;

        QMetaObject::invokeMethod(s_previewerServer, "send", Qt::QueuedConnection,
                                  Q_ARG(PreviewerCommands, Init));
    }
}

void ControlPreviewingManager::onDisconnected()
{
    // TODO
    qDebug() << "Connection lost in" << __FILE__ << ":" << __LINE__;
}

void ControlPreviewingManager::onConnectionTimeout()
{
    // TODO
    qDebug() << "Connection timeout in" << __FILE__ << ":" << __LINE__;
}