#include <applicationcore.h>
#include <toolmanager.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <projectexposingmanager.h>
#include <controlcreationmanager.h>
#include <windowmanager.h>
#include <authenticator.h>
#include <controlpreviewingmanager.h>
#include <controltransactionmanager.h>
#include <documentmanager.h>
#include <mainwindow.h>
#include <controlpreviewingmanager.h>
#include <runmanager.h>
#include <controlcreationmanager.h>
#include <savemanager.h>
#include <controlmonitoringmanager.h>
#include <menumanager.h>
#include <centralwidget.h>
#include <designerwidget.h>
#include <controlremovingmanager.h>
#include <welcomewindow.h>

#include <QMessageBox>

#include <coreplugin/coreconstants.h>
#include <theme/theme_p.h>
#include <coreplugin/themechooser.h>
#include <coreplugin/helpmanager.h>

using namespace Core;

ApplicationCore* ApplicationCore::s_instance = nullptr;
Authenticator* ApplicationCore::s_authenticator = nullptr;
UserManager* ApplicationCore::s_userManager = nullptr;
ControlPreviewingManager* ApplicationCore::s_controlPreviewingManager = nullptr;
SaveManager* ApplicationCore::s_saveManager = nullptr;
ProjectManager* ApplicationCore::s_projectManager = nullptr;
ProjectExposingManager* ApplicationCore::s_projectExposingManager = nullptr;
ControlCreationManager* ApplicationCore::s_controlExposingManager = nullptr;
ControlRemovingManager* ApplicationCore::s_controlRemovingManager = nullptr;
RunManager* ApplicationCore::s_runManager = nullptr;
ControlMonitoringManager* ApplicationCore::s_controlMonitoringManager = nullptr;
ControlTransactionManager* ApplicationCore::s_controlTransactionManager = nullptr;
HelpManager* ApplicationCore::s_helpManager = nullptr;
DocumentManager* ApplicationCore::s_documentManager = nullptr;
WindowManager* ApplicationCore::s_windowManager = nullptr;
MenuManager* ApplicationCore::s_menuManager = nullptr;

ApplicationCore::ApplicationCore(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_authenticator = new Authenticator(this);
    s_userManager = new UserManager(this);
    s_controlPreviewingManager = new ControlPreviewingManager(this);
    s_saveManager = new SaveManager(this);
    s_projectManager = new ProjectManager(this);
    s_projectExposingManager = new ProjectExposingManager(this);
    s_controlExposingManager = new ControlCreationManager(this);
    s_controlRemovingManager = new ControlRemovingManager(this);
    s_runManager = new RunManager(this);
    s_controlMonitoringManager = new ControlMonitoringManager(this);
    s_controlTransactionManager = new ControlTransactionManager(this);
    s_helpManager = new HelpManager(this);

    HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Internal::ThemeEntry::createTheme(Constants::DEFAULT_THEME));
    connect(qApp, &QCoreApplication::aboutToQuit, s_helpManager, &HelpManager::aboutToShutdown);

    s_documentManager = new DocumentManager(this);

    connect(UserManager::instance(), &UserManager::aboutToStop,
            this, &ApplicationCore::onSessionStop);

    Authenticator::setHost(QUrl(APP_WSSSERVER));

    if (!ControlPreviewingManager::init()) {
        QMessageBox::critical(nullptr,
                              tr("Error"),
                              tr("Unable to start Objectwheel Previewing Service"));
    }

    //! GUI initialization starts here
    s_windowManager = new WindowManager(this);
    s_menuManager = new MenuManager(this);

    connect(ProjectManager::instance(), &ProjectManager::stopped,
            WindowManager::mainWindow(), &MainWindow::sweep);
    ProjectExposingManager::init(
                WindowManager::mainWindow()->centralWidget()->designerWidget()->designerScene());
    ControlCreationManager::init(
                WindowManager::mainWindow()->centralWidget()->designerWidget()->designerScene());
    ControlRemovingManager::init(
                WindowManager::mainWindow()->centralWidget()->designerWidget()->designerScene());

    // Show welcome window
    WindowManager::welcomeWindow()->show();
}

ApplicationCore::~ApplicationCore()
{
    s_instance = nullptr;
}

void ApplicationCore::init(QObject* parent)
{
    static ApplicationCore* instance = nullptr;
    if (instance == nullptr)
        instance = new ApplicationCore(parent);
}

ApplicationCore* ApplicationCore::instance()
{
    return s_instance;
}

QSettings* ApplicationCore::settings(QSettings::Scope scope)
{
    static QSettings settings, globalSettings;
    return scope == QSettings::UserScope ? &settings : &globalSettings;
}

QString ApplicationCore::resourcePath()
{
    return tr(":");
}

QString ApplicationCore::userResourcePath()
{
    return tr(":");
}

void ApplicationCore::onSessionStop()
{
    ProjectManager::stop();
}
