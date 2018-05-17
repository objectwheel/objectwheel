#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <exposerbackend.h>
#include <windowmanager.h>
#include <authenticator.h>
#include <previewerbackend.h>
#include <savetransaction.h>
#include <editorbackend.h>
#include <QMessageBox>
#include <coreplugin/coreconstants.h>

EditorBackend* BackendManager::m_editorBackend = nullptr;

BackendManager::BackendManager()
{
    Core::HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Core::Internal::ThemeEntry::createTheme(Core::Constants::DEFAULT_THEME));
    connect(qApp, &QCoreApplication::aboutToQuit, &m_helpManager, &Core::HelpManager::aboutToShutdown);

    SaveTransaction::instance();
    Authenticator::instance()->init(QUrl(APP_WSSSERVER));

    if (!PreviewerBackend::instance()->init()) {
        QMessageBox::critical(
            nullptr,
            tr("Error"),
            tr("Unable to start Objectwheel Previewing Service")
        );
    }
}

BackendManager::~BackendManager()
{
    delete m_editorBackend;
}

BackendManager* BackendManager::instance()
{
    static BackendManager instance;
    return &instance;
}

void BackendManager::init()
{
    m_editorBackend = new EditorBackend;

    connect(ProjectBackend::instance(), SIGNAL(started()),
            WindowManager::instance()->get(WindowManager::Main), SLOT(reset()));
    connect(ProjectBackend::instance(), SIGNAL(started()),
            instance(), SLOT(handleProjectStart()));
    connect(UserBackend::instance(), SIGNAL(aboutToStop()),
            instance(), SLOT(handleSessionStop()));
}

QSettings* BackendManager::settings(QSettings::Scope scope)
{
    static QSettings settings, globalSettings;
    return scope == QSettings::UserScope ? &settings : &globalSettings;
}

QString BackendManager::resourcePath()
{
    return tr(":");
}

QString BackendManager::userResourcePath()
{
    return tr(":");
}

void BackendManager::handleSessionStop() const
{
    ProjectBackend::instance()->stop();
}

void BackendManager::handleProjectStart() const
{
    PreviewerBackend::instance()->restart();
    PreviewerBackend::instance()->requestInit(ProjectBackend::instance()->dir());
    ExposerBackend::instance()->exposeProject();
//    dW->controlScene()->clearSelection();
//    dW->designerScene()->clearSelection();
    ToolsBackend::instance()->downloadTools();
    // MainWindow::instance()->setWindowTitle //FIXME
    // (QString(APP_NAME) + " - [" + projectname + "]");
}
