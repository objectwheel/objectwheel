#include <controlpreviewingmanager.h>
#include <projectmanager.h>
#include <hashfactory.h>
#include <previewerserver.h>

#include <QProcess>
#include <QCoreApplication>

namespace {
bool initScheduled = false;
QString serverName;
}

ControlPreviewingManager* ControlPreviewingManager::s_instance = nullptr;
PreviewerServer* ControlPreviewingManager::s_previewerServer = nullptr;

ControlPreviewingManager::ControlPreviewingManager(QObject *parent) : QObject(parent)
{
    s_instance = this;
    serverName = HashFactory::generate();

    s_previewerServer = new PreviewerServer(this);
    connect(s_previewerServer, &PreviewerServer::connected,
            this, &ControlPreviewingManager::onConnected);

#if defined(PREVIEWER_DEBUG)
    QLocalServer::removeServer("serverName");
    s_previewerServer->listen("serverName");
#else
    s_previewerServer->listen(serverName);
#endif
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
    QStringList arguments;
    arguments << ProjectManager::dir();
    arguments << serverName;

    QProcess process;
    process.setArguments(arguments);
    process.setProgram(QCoreApplication::applicationDirPath() + "/previewer");
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    process.startDetached();

    initScheduled = true;
}

void ControlPreviewingManager::onConnected()
{
    if (initScheduled) {
        initScheduled = false;
        s_previewerServer->send(PreviewerCommands::Init);
    }
}