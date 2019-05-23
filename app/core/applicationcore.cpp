#include <applicationcore.h>
#include <toolmanager.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <projectexposingmanager.h>
#include <controlcreationmanager.h>
#include <windowmanager.h>
#include <registrationapimanager.h>
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
#include <splashscreen.h>
#include <helpmanager.h>
#include <globalresources.h>
#include <components.h>
#include <paintutils.h>
#include <servermanager.h>

#include <QStandardPaths>
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
GlobalResources* ApplicationCore::s_globalResources = nullptr;
ServerManager* ApplicationCore::s_serverManager = nullptr;
RegistrationApiManager* ApplicationCore::s_accountManager = nullptr;
UserManager* ApplicationCore::s_userManager = nullptr;
ControlPreviewingManager* ApplicationCore::s_controlPreviewingManager = nullptr;
SaveManager* ApplicationCore::s_saveManager = nullptr;
ProjectManager* ApplicationCore::s_projectManager = nullptr;
ProjectExposingManager* ApplicationCore::s_projectExposingManager = nullptr;
ControlCreationManager* ApplicationCore::s_controlCreationManager = nullptr;
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

    QApplication::setPalette(palette());
//    QApplication::setPalette(PaintUtils::defaultButtonPalette(), "ModeSelectorPane");
    QApplication::setPalette(PaintUtils::defaultButtonPalette(), "PushButton");
    QApplication::setPalette(PaintUtils::defaultButtonPalette(), "RunProgressBar");
    QApplication::setPalette(PaintUtils::defaultButtonPalette(), "RunDevicesButton");
    QApplication::setPalette(PaintUtils::defaultButtonPalette(), "FocuslessLineEdit");

    const QString fontPath = ":/fonts";
    const QString settingsPath = QApplication::applicationDirPath() + "/settings.ini";

    /* Load default fonts */
    for (const QString& fontName : QDir(fontPath).entryList(QDir::Files))
        QFontDatabase::addApplicationFont(fontPath + '/' + fontName);

    /* Prepare setting instances */
    s_settings = new QSettings(settingsPath, QSettings::IniFormat, app);
    s_generalSettings = new GeneralSettings(app);
    s_codeEditorSettings = new CodeEditorSettings(app);

    /* Read settings */
    GeneralSettings::read();
    CodeEditorSettings::read();

    /* Set application ui settings */
    QApplication::setFont(GeneralSettings::interfaceSettings()->toFont());
    QApplication::setStyle(new ApplicationStyle); // Ownership taken by QApplication

    /* Show splash screen */
    SplashScreen splashScreen;
    Q_UNUSED(splashScreen);

    s_serverManager = new ServerManager(QUrl(APP_WSSSERVER), app);
    s_accountManager = new RegistrationApiManager(app);
    s_userManager = new UserManager(app);
    s_controlPreviewingManager = new ControlPreviewingManager(app);
    s_saveManager = new SaveManager(app);
    s_projectManager = new ProjectManager(app);
    s_globalResources = new GlobalResources([=] () -> QString { return ProjectManager::dir(); }, app);
    s_projectExposingManager = new ProjectExposingManager(app);
    s_controlCreationManager = new ControlCreationManager(app);
    s_controlRemovingManager = new ControlRemovingManager(app);
    s_controlPropertyManager = new ControlPropertyManager(app);
    s_runManager = new RunManager(app);
    s_helpManager = new HelpManager(app);

    QObject::connect(s_serverManager, &ServerManager::dataArrived,
                     s_accountManager, &RegistrationApiManager::onDataArrival);
    s_serverManager->start();

    s_helpManager->setupHelpManager();
    Utils::setCreatorTheme(Core::Internal::ThemeEntry::createTheme(Core::Constants::DEFAULT_THEME));
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     s_helpManager, &HelpManager::aboutToShutdown);

    s_documentManager = new DocumentManager(app);

    Components::init();

    /** Ui initialization **/
    s_windowManager = new WindowManager(app);
    s_menuManager = new MenuManager(app);
//  FIXME  QObject::connect(s_userManager, &UserManager::started,
//                     &ApplicationCore::onUserSessionStart);
//    QObject::connect(s_userManager, &UserManager::aboutToStop,
//                     &ApplicationCore::onUserSessionStop);
    QObject::connect(s_projectManager, &ProjectManager::started,
                     &ApplicationCore::onProjectStart);
    QObject::connect(s_projectManager, &ProjectManager::stopped,
                     &ApplicationCore::onProjectStop);

    DesignerScene* scene = s_windowManager->mainWindow()->centralWidget()->designerWidget()->designerScene();
    s_projectExposingManager->init(scene);
    s_controlCreationManager->init(scene);
    s_controlRemovingManager->init(scene);
    s_windowManager->welcomeWindow()->show();
}

bool ApplicationCore::locked()
{
    QSharedMemory* sharedMemory = new QSharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory->create(1)) {
        sharedMemory->attach();
        sharedMemory->detach();
        if(!sharedMemory->create(1)) {
            UtilityFunctions::showMessage(nullptr,
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
    // qputenv("QT_SCALE_FACTOR", "2");
    const QString settingsPath = QFileInfo(filePath).path() + "/settings.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);
    if (settings.value("General/Interface.HdpiEnabled", InterfaceSettings().hdpiEnabled).toBool())
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
    const QString settingsPath = QApplication::applicationDirPath() + "/settings.ini";
    QPalette palette(QApplication::palette());
    QSettings settings(settingsPath, QSettings::IniFormat);
    if (settings.value("General/Interface.Theme", InterfaceSettings().theme).toString() == "Light") {
        palette.setColor(QPalette::Active, QPalette::Text, "#272727");
        palette.setColor(QPalette::Inactive, QPalette::Text, "#272727");
        palette.setColor(QPalette::Disabled, QPalette::Text, "#6f7e8c");
        palette.setColor(QPalette::Active, QPalette::WindowText, "#272727");
        palette.setColor(QPalette::Inactive, QPalette::WindowText, "#272727");
        palette.setColor(QPalette::Disabled, QPalette::WindowText, "#6f7e8c");
        palette.setColor(QPalette::Active, QPalette::ButtonText, "#272727");
        palette.setColor(QPalette::Inactive, QPalette::ButtonText, "#272727");
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#6f7e8c");
        palette.setColor(QPalette::Active, QPalette::PlaceholderText, "#bfbfbf");
        palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, "#bfbfbf");
        palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, "#bfbfbf");
        palette.setColor(QPalette::Base, "#ffffff");
        palette.setColor(QPalette::Button, "#f0f0f0");
        palette.setColor(QPalette::Window, "#f0f0f0");
        palette.setColor(QPalette::Highlight, "#1C77D5");
        palette.setColor(QPalette::BrightText, "#ffffff");
        palette.setColor(QPalette::HighlightedText, "#ffffff");
        palette.setColor(QPalette::ToolTipText, "#272727");
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
        palette.setColor(QPalette::Active, QPalette::PlaceholderText, "#666666");
        palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, "#666666");
        palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, "#666666");
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

QString ApplicationCore::resourcePath()
{
    return ":";
}

QString ApplicationCore::appDataLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
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
