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
#include <generalsettings.h>
#include <interfacesettings.h>
#include <codeeditorsettings.h>
#include <applicationstyle.h>
#include <filemanager.h>
#include <splashscreen.h>
#include <helpmanager.h>

#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QFontDatabase>
#include <QSharedMemory>

#include <theme/theme_p.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/themechooser.h>

QSettings* ApplicationCore::s_settings = nullptr;
GeneralSettings* ApplicationCore::s_generalSettings = nullptr;
CodeEditorSettings* ApplicationCore::s_codeEditorSettings = nullptr;
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

ApplicationCore::ApplicationCore(QApplication* app)
{
    /** Core initialization **/
    QApplication::setApplicationName(APP_NAME);
    QApplication::setOrganizationName(APP_CORP);
    QApplication::setApplicationVersion(APP_VER);
    QApplication::setOrganizationDomain(APP_DOMAIN);
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setWindowIcon(QIcon(":/images/owicon.png"));

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    QFont font(settings->fontFamily);
    font.setPixelSize(settings->fontPixelSize);
    font.setWeight(settings->fontPreferThick ? QFont::DemiBold : QFont::Normal);
    font.setStyleStrategy(settings->fontPreferAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);

    /* Set application ui settings */
    QApplication::setFont(font);
    QApplication::setPalette(palette());
    QApplication::setStyle(new ApplicationStyle); // Ownership taken by QApplication

    /* Show splash screen */
    SplashScreen splashScreen;
    Q_UNUSED(splashScreen);

    s_authenticator = new Authenticator(app);
    s_userManager = new UserManager(app);
    s_controlPreviewingManager = new ControlPreviewingManager(app);
    s_saveManager = new SaveManager(app);
    s_projectManager = new ProjectManager(app);
    s_projectExposingManager = new ProjectExposingManager(app);
    s_controlExposingManager = new ControlCreationManager(app);
    s_controlRemovingManager = new ControlRemovingManager(app);
    s_controlPropertyManager = new ControlPropertyManager(app);
    s_runManager = new RunManager(app);
    s_helpManager = new HelpManager(app);

    HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Core::Internal::ThemeEntry::createTheme(Core::Constants::DEFAULT_THEME));
    QObject::connect(QApplication::instance(), &QCoreApplication::aboutToQuit,
                     s_helpManager, &HelpManager::aboutToShutdown);

    s_documentManager = new DocumentManager(app);

    Authenticator::setHost(QUrl(APP_WSSSERVER));

    /** Ui initialization **/
    s_windowManager = new WindowManager(app);
    s_menuManager = new MenuManager(app);

    QObject::connect(UserManager::instance(), &UserManager::started,
                     &ApplicationCore::onUserSessionStart);
    QObject::connect(UserManager::instance(), &UserManager::aboutToStop,
                     &ApplicationCore::onUserSessionStop);
    QObject::connect(ProjectManager::instance(), &ProjectManager::started,
                     &ApplicationCore::onProjectStart);
    QObject::connect(ProjectManager::instance(), &ProjectManager::stopped,
                     &ApplicationCore::onProjectStop);

    DesignerScene* scene = WindowManager::mainWindow()->centralWidget()->designerWidget()->designerScene();
    ProjectExposingManager::init(scene);
    ControlCreationManager::init(scene);
    ControlRemovingManager::init(scene);

    // Show welcome window
    WindowManager::welcomeWindow()->show();
}

bool ApplicationCore::locked()
{
    QSharedMemory* sharedMemory = new QSharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory->create(1)) {
        sharedMemory->attach();
        sharedMemory->detach();
        if(!sharedMemory->create(1)) {
            QMessageBox::warning(nullptr,
                                 QObject::tr("Quitting"),
                                 QObject::tr("Another instance is already running."));
            return true;
        }
    }
    return false;
}

void ApplicationCore::run(QApplication* app)
{
    static ApplicationCore instance(app);
    Q_UNUSED(instance);
}

void ApplicationCore::prepare(const char* filePath)
{
    const QString fontPath = ":/fonts";
    const QString settingsPath = dname(filePath) + "/settings.ini";

    /* Prepare setting instances */
    s_settings = new QSettings(settingsPath, QSettings::IniFormat, nullptr);
    s_generalSettings = new GeneralSettings(nullptr);
    s_codeEditorSettings = new CodeEditorSettings(nullptr);

    /* Read settings */
    GeneralSettings::read();
    CodeEditorSettings::read();

    /* Load default fonts */
    for (const QString& fontName : lsfile(fontPath))
        QFontDatabase::addApplicationFont(fontPath + fontName);

    if (GeneralSettings::interfaceSettings()->hdpiEnabled)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules("qtc*.info=false\n"
                                     "qtc*.debug=false\n"
                                     "qtc*.warning=false\n"
                                     "qtc*.critical=false\n"
                                     "qtc*=false");
}

QPalette ApplicationCore::palette()
{
    QPalette palette(QApplication::palette());
    if (GeneralSettings::interfaceSettings()->theme == "Light") {
        palette.setColor(QPalette::Active, QPalette::Text, "#3C444C");
        palette.setColor(QPalette::Inactive, QPalette::Text, "#3C444C");
        palette.setColor(QPalette::Disabled, QPalette::Text, "#6f7e8c");
        palette.setColor(QPalette::Active, QPalette::WindowText, "#3C444C");
        palette.setColor(QPalette::Inactive, QPalette::WindowText, "#3C444C");
        palette.setColor(QPalette::Disabled, QPalette::WindowText, "#6f7e8c");
        palette.setColor(QPalette::Active, QPalette::ButtonText, "#3C444C");
        palette.setColor(QPalette::Inactive, QPalette::ButtonText, "#3C444C");
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#6f7e8c");
        palette.setColor(QPalette::Base, "#ffffff");
        palette.setColor(QPalette::Button, "#f0f0f0");
        palette.setColor(QPalette::Window, "#f0f0f0");
        palette.setColor(QPalette::Highlight, "#1C77D5");
        palette.setColor(QPalette::BrightText, "#ffffff");
        palette.setColor(QPalette::HighlightedText, "#ffffff");
        palette.setColor(QPalette::ToolTipText, "#3C444C");
        palette.setColor(QPalette::ToolTipBase, "#f0f0f0");
        palette.setColor(QPalette::Link, "#025dbf");
        palette.setColor(QPalette::LinkVisited, "#B44B46");
    } else {
        palette.setColor(QPalette::Active, QPalette::Text, "#e5e5e5");
        palette.setColor(QPalette::Inactive, QPalette::Text, "#e5e5e5");
        palette.setColor(QPalette::Disabled, QPalette::Text, "#e5e5e5");
        palette.setColor(QPalette::Active, QPalette::WindowText, "#e5e5e5");
        palette.setColor(QPalette::Inactive, QPalette::WindowText, "#e5e5e5");
        palette.setColor(QPalette::Disabled, QPalette::WindowText, "#b0b0b0");
        palette.setColor(QPalette::Active, QPalette::ButtonText, "#e5e5e5");
        palette.setColor(QPalette::Inactive, QPalette::ButtonText, "#e5e5e5");
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#b0b0b0");
        palette.setColor(QPalette::Base, "#444e57");
        palette.setColor(QPalette::Button, "#444e57");
        palette.setColor(QPalette::Window, "#2F363C");
        palette.setColor(QPalette::Highlight, "#1C77D5");
        palette.setColor(QPalette::BrightText, "#e5e5e5");
        palette.setColor(QPalette::HighlightedText, "#e5e5e5");
        palette.setColor(QPalette::ToolTipText, "#e5e5e5");
        palette.setColor(QPalette::ToolTipBase, "#2F363C");
        palette.setColor(QPalette::Link, "#025dbf");
        palette.setColor(QPalette::LinkVisited, "#B44B46");
    }
    return palette;
}

QSettings* ApplicationCore::settings()
{
    return s_settings;
}

const char* ApplicationCore::resourcePath()
{
    return ":";
}

const char* ApplicationCore::userResourcePath()
{
    return ":";
}

void ApplicationCore::onProjectStop()
{
    WindowManager::mainWindow()->discharge();
}

void ApplicationCore::onProjectStart()
{
    // WindowManager::mainWindow()->discharge();
}

void ApplicationCore::onUserSessionStop()
{
    ProjectManager::stop();
}

void ApplicationCore::onUserSessionStart()
{
    // DocumentManager::load();
}
