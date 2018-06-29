#include <controlpreviewingmanager.h>
#include <projectmanager.h>
#include <previewerserver.h>
#include <commanddispatcher.h>
#include <hashfactory.h>

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QCoreApplication>
#include <QLocalServer>

namespace { bool g_initScheduled = false; }

ControlPreviewingManager* ControlPreviewingManager::s_instance = nullptr;
PreviewerServer* ControlPreviewingManager::s_previewerServer = nullptr;
QThread* ControlPreviewingManager::s_serverThread = nullptr;
CommandDispatcher* ControlPreviewingManager::s_commandDispatcher = nullptr;

ControlPreviewingManager::ControlPreviewingManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<PreviewerCommands>("PreviewerCommands");

    s_instance = this;
    s_previewerServer = new PreviewerServer;
    s_serverThread = new QThread(this);

    s_previewerServer->moveToThread(s_serverThread);
    s_serverThread->start();

    s_commandDispatcher = new CommandDispatcher(s_previewerServer, this);

    connect(s_previewerServer, &PreviewerServer::connected,
            this, &ControlPreviewingManager::onConnected);

    connect(s_previewerServer, &PreviewerServer::disconnected,
            this, &ControlPreviewingManager::onDisconnected);
    connect(s_previewerServer, &PreviewerServer::connectionTimeout,
            this, &ControlPreviewingManager::onConnectionTimeout);

    connect(s_commandDispatcher, &CommandDispatcher::initializationProgressChanged,
            this, &ControlPreviewingManager::initializationProgressChanged);

#if defined(PREVIEWER_DEBUG)
    QLocalServer::removeServer("serverName");
    QMetaObject::invokeMethod(s_previewerServer, "listen", Q_ARG(QString, "serverName"));
#else
    QMetaObject::invokeMethod(s_previewerServer, "listen", Q_ARG(QString, HashFactory::generate()));
#endif
}

ControlPreviewingManager::~ControlPreviewingManager()
{
    scheduleTerminate();

    QMetaObject::invokeMethod(s_previewerServer, "deleteLater");

    s_serverThread->quit();
    s_serverThread->wait();

    s_instance = nullptr;
}

ControlPreviewingManager* ControlPreviewingManager::instance()
{
    return s_instance;
}

void ControlPreviewingManager::schedulePreview(const QString& uid)
{
    s_commandDispatcher->schedulePreview(uid);
}

void ControlPreviewingManager::scheduleInit()
{
    s_serverThread->wait(100);

    if (g_initScheduled || s_previewerServer->isConnected()) {
        qWarning() << "Terminate existing connection first in" << __FILE__ << ":" << __LINE__;
        return;
    }

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

#if !defined(PREVIEWER_DEBUG)
    process.startDetached();
#endif

    g_initScheduled = true;
}

void ControlPreviewingManager::scheduleTerminate()
{
    s_commandDispatcher->scheduleTerminate();
    QMetaObject::invokeMethod(s_previewerServer, "abort");
}

void ControlPreviewingManager::onConnected()
{
    if (g_initScheduled) {
        g_initScheduled = false;

        s_commandDispatcher->scheduleInit();
    }
}

void ControlPreviewingManager::onDisconnected()
{
    // TODO
    qWarning() << "Connection lost in" << __FILE__ << ":" << __LINE__;
}

void ControlPreviewingManager::onConnectionTimeout()
{
    // TODO
    qWarning() << "Connection timeout in" << __FILE__ << ":" << __LINE__;
}