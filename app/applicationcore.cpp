#include <applicationcore.h>
#include <toolmanager.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <projectexposingmanager.h>
#include <controlcreationmanager.h>
#include <windowmanager.h>
#include <authenticator.h>
#include <controlpreviewingmanager.h>
#include <documentmanager.h>
#include <mainwindow.h>
#include <runmanager.h>
#include <controlcreationmanager.h>
#include <savemanager.h>
#include <menumanager.h>
#include <centralwidget.h>
#include <designerwidget.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <welcomewindow.h>
#include <appfontsettings.h>

#include <QApplication>

#include <theme/theme_p.h>
#include <coreplugin/coreconstants.h>
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
ControlPropertyManager* ApplicationCore::s_controlPropertyManager = nullptr;
RunManager* ApplicationCore::s_runManager = nullptr;
HelpManager* ApplicationCore::s_helpManager = nullptr;
DocumentManager* ApplicationCore::s_documentManager = nullptr;
WindowManager* ApplicationCore::s_windowManager = nullptr;
MenuManager* ApplicationCore::s_menuManager = nullptr;

ApplicationCore::ApplicationCore(QObject* parent) : QObject(parent)
{
    //! Core initialization
    QApplication::setApplicationName(APP_NAME);
    QApplication::setOrganizationName(APP_CORP);
    QApplication::setApplicationVersion(APP_VER);
    QApplication::setOrganizationDomain(APP_DOMAIN);
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setWindowIcon(QIcon(":/images/owicon.png"));

    /* Set Font */
    AppFontSettings::apply();

    s_authenticator = new Authenticator(this);
    s_userManager = new UserManager(this);
    s_controlPreviewingManager = new ControlPreviewingManager(this);
    s_saveManager = new SaveManager(this);
    s_projectManager = new ProjectManager(this);
    s_projectExposingManager = new ProjectExposingManager(this);
    s_controlExposingManager = new ControlCreationManager(this);
    s_controlRemovingManager = new ControlRemovingManager(this);
    s_controlPropertyManager = new ControlPropertyManager(this);
    s_runManager = new RunManager(this);
    s_helpManager = new HelpManager(this);

    HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Internal::ThemeEntry::createTheme(Constants::DEFAULT_THEME));
    connect(qApp, &QCoreApplication::aboutToQuit, s_helpManager, &HelpManager::aboutToShutdown);

    QPalette palette(QApplication::palette());
    palette.setColor(QPalette::Window, "#f0f0f0");
    palette.setColor(QPalette::Highlight, "#4278f5");
    palette.setColor(QPalette::ToolTipBase, "#E9E9E9");
    palette.setColor(QPalette::ToolTipText, "#222222");
    QApplication::setPalette(palette);

    s_documentManager = new DocumentManager(this);

    connect(UserManager::instance(), &UserManager::aboutToStop,
            this, &ApplicationCore::onSessionStop);

    Authenticator::setHost(QUrl(APP_WSSSERVER));

    //! GUI initialization
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

void ApplicationCore::init(QObject* parent)
{
    if (s_instance)
        return;

    s_instance = new ApplicationCore(parent);
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
