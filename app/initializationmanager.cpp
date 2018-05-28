#include <initializationmanager.h>
#include <toolmanager.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <controlexposingmanager.h>
#include <windowmanager.h>
#include <authenticator.h>
#include <controlpreviewingmanager.h>
#include <controltransactionmanager.h>
#include <documentmanager.h>
#include <mainwindow.h>
#include <controlpreviewingmanager.h>
#include <runmanager.h>
#include <controlexposingmanager.h>
#include <savemanager.h>
#include <controlmonitoringmanager.h>
#include <menumanager.h>
#include <centralwidget.h>
#include <designerwidget.h>

#include <QMessageBox>

#include <coreplugin/coreconstants.h>
#include <theme/theme_p.h>
#include <coreplugin/themechooser.h>
#include <coreplugin/helpmanager.h>

using namespace Core;

InitializationManager* InitializationManager::s_instance = nullptr;
Authenticator* InitializationManager::s_authenticator = nullptr;
UserManager* InitializationManager::s_userManager = nullptr;
ControlPreviewingManager* InitializationManager::s_previewerManager = nullptr;
SaveManager* InitializationManager::s_saveManager = nullptr;
ProjectManager* InitializationManager::s_projectManager = nullptr;
ControlExposingManager* InitializationManager::s_exposerManager = nullptr;
RunManager* InitializationManager::s_interpreterManager = nullptr;
ControlMonitoringManager* InitializationManager::s_controlWatcher = nullptr;
ControlTransactionManager* InitializationManager::s_saveTransaction = nullptr;
HelpManager* InitializationManager::s_helpManager = nullptr;
DocumentManager* InitializationManager::s_editorManager = nullptr;
WindowManager* InitializationManager::s_windowManager = nullptr;
MenuManager* InitializationManager::s_menuManager = nullptr;

InitializationManager::InitializationManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_authenticator = new Authenticator(this);
    s_userManager = new UserManager(this);
    s_previewerManager = new ControlPreviewingManager(this);
    s_saveManager = new SaveManager(this);
    s_projectManager = new ProjectManager(this);
    s_exposerManager = new ControlExposingManager;
    s_interpreterManager = new RunManager(this);
    s_controlWatcher = new ControlMonitoringManager(this);
    s_saveTransaction = new ControlTransactionManager(this);
    s_helpManager = new HelpManager(this);

    HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Internal::ThemeEntry::createTheme(Constants::DEFAULT_THEME));
    connect(qApp, &QCoreApplication::aboutToQuit, s_helpManager, &HelpManager::aboutToShutdown);

    s_editorManager = new DocumentManager(this);

    connect(UserManager::instance(), &UserManager::aboutToStop,
            this, &InitializationManager::onSessionStop);

    Authenticator::setHost(QUrl(APP_WSSSERVER));

    if (!ControlPreviewingManager::init()) {
        QMessageBox::critical(
                    nullptr,
                    tr("Error"),
                    tr("Unable to start Objectwheel Previewing Service")
                    );
    }

    //! GUI initialization starts here
    s_windowManager = new WindowManager(this);
    s_menuManager = new MenuManager(this);

    connect(ProjectManager::instance(), &ProjectManager::started,
            WindowManager::mainWindow(), &MainWindow::reset);
    connect(ProjectManager::instance(), &ProjectManager::started,
            InitializationManager::instance(), &InitializationManager::onProjectStart);

    ControlExposingManager::init(
                WindowManager::mainWindow()->centralWidget()->designerWidget()->designerScene());
}

InitializationManager::~InitializationManager()
{
    delete s_exposerManager;
    s_instance = nullptr;
}

void InitializationManager::init(QObject* parent)
{
    static InitializationManager* instance = nullptr;
    if (instance == nullptr)
        instance = new InitializationManager(parent);
}

InitializationManager* InitializationManager::instance()
{
    return s_instance;
}

QSettings* InitializationManager::settings(QSettings::Scope scope)
{
    static QSettings settings, globalSettings;
    return scope == QSettings::UserScope ? &settings : &globalSettings;
}

QString InitializationManager::resourcePath()
{
    return tr(":");
}

QString InitializationManager::userResourcePath()
{
    return tr(":");
}

void InitializationManager::onSessionStop()
{
    ProjectManager::stop();
}

void InitializationManager::onProjectStart()
{
    ControlPreviewingManager::restart();
    ControlPreviewingManager::requestInit(ProjectManager::dir());
    ControlExposingManager::exposeProject();
    //    dW->controlScene()->clearSelection();
    //    dW->designerScene()->clearSelection();
    ToolManager::downloadTools();
    // MainWindow::instance()->setWindowTitle //FIXME
    // (QString(APP_NAME) + " - [" + projectname + "]");
}
