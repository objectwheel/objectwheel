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
#include <mainwindow.h>
#include <previewerbackend.h>
#include <interpreterbackend.h>
#include <exposerbackend.h>
#include <savebackend.h>
#include <controlwatcher.h>
#include <menumanager.h>

#include <QMessageBox>

#include <coreplugin/coreconstants.h>
#include <theme/theme_p.h>
#include <coreplugin/themechooser.h>
#include <coreplugin/helpmanager.h>

using namespace Core;

BackendManager* BackendManager::s_instance = nullptr;
Authenticator* BackendManager::s_authenticator = nullptr;
UserBackend* BackendManager::s_userBackend = nullptr;
PreviewerBackend* BackendManager::s_previewerBackend = nullptr;
SaveBackend* BackendManager::s_saveBackend = nullptr;
ProjectBackend* BackendManager::s_projectBackend = nullptr;
ExposerBackend* BackendManager::s_exposerBackend = nullptr;
InterpreterBackend* BackendManager::s_interpreterBackend = nullptr;
ControlWatcher* BackendManager::s_controlWatcher = nullptr;
SaveTransaction* BackendManager::s_saveTransaction = nullptr;
HelpManager* BackendManager::s_helpManager = nullptr;
EditorBackend* BackendManager::s_editorBackend = nullptr;
WindowManager* BackendManager::s_windowManager = nullptr;
MenuManager* BackendManager::s_menuManager = nullptr;

BackendManager::BackendManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_authenticator = new Authenticator(this);
    s_userBackend = new UserBackend(this);
    s_previewerBackend = new PreviewerBackend(this);
    s_saveBackend = new SaveBackend(this);
    s_projectBackend = new ProjectBackend(this);
    s_exposerBackend = new ExposerBackend;
    s_interpreterBackend = new InterpreterBackend(this);
    s_controlWatcher = new ControlWatcher(this);
    s_saveTransaction = new SaveTransaction(this);
    s_helpManager = new HelpManager(this);

    HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Internal::ThemeEntry::createTheme(Constants::DEFAULT_THEME));
    connect(qApp, &QCoreApplication::aboutToQuit, s_helpManager, &HelpManager::aboutToShutdown);

    s_editorBackend = new EditorBackend(this);

    connect(UserBackend::instance(), &UserBackend::aboutToStop,
            this, &BackendManager::onSessionStop);

    Authenticator::setHost(QUrl(APP_WSSSERVER));

    if (!PreviewerBackend::init()) {
        QMessageBox::critical(
            nullptr,
            tr("Error"),
            tr("Unable to start Objectwheel Previewing Service")
        );
    }

    s_windowManager = new WindowManager(this);
    s_menuManager = new MenuManager(this);

    connect(ProjectBackend::instance(), &ProjectBackend::started,
            WindowManager::mainWindow(), &MainWindow::reset);
    connect(ProjectBackend::instance(), &ProjectBackend::started,
            BackendManager::instance(), &BackendManager::onProjectStart);
}

BackendManager::~BackendManager()
{
    delete s_exposerBackend;
    s_instance = nullptr;
}

void BackendManager::init(QObject* parent)
{
    static BackendManager* instance = nullptr;
    if (instance == nullptr)
        instance = new BackendManager(parent);
}

BackendManager* BackendManager::instance()
{
    return s_instance;
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

void BackendManager::onSessionStop()
{
    ProjectBackend::stop();
}

void BackendManager::onProjectStart()
{
    PreviewerBackend::restart();
    PreviewerBackend::requestInit(ProjectBackend::dir());
    ExposerBackend::exposeProject();
//    dW->controlScene()->clearSelection();
//    dW->designerScene()->clearSelection();
    ToolsBackend::downloadTools();
    // MainWindow::instance()->setWindowTitle //FIXME
    // (QString(APP_NAME) + " - [" + projectname + "]");
}
