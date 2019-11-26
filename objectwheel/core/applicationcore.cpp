#include <applicationcore.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <projectexposingmanager.h>
#include <controlcreationmanager.h>
#include <windowmanager.h>
#include <registrationapimanager.h>
#include <controlrenderingmanager.h>
#include <documentmanager.h>
#include <mainwindow.h>
#include <runmanager.h>
#include <controlcreationmanager.h>
#include <savemanager.h>
#include <menumanager.h>
#include <centralwidget.h>
#include <designerpane.h>
#include <designerview.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <welcomewindow.h>
#include <generalsettings.h>
#include <designersettings.h>
#include <interfacesettings.h>
#include <codeeditorsettings.h>
#include <applicationstyle.h>
#include <helpmanager.h>
#include <paintutils.h>
#include <servermanager.h>
#include <modemanager.h>

#include <QScreen>
#include <QStandardPaths>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QFontDatabase>
#include <QSharedMemory>
#include <QSplashScreen>
#include <QTimer>

#include <theme/theme_p.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/themechooser.h>

QSettings* ApplicationCore::s_settings = nullptr;
GeneralSettings* ApplicationCore::s_generalSettings = nullptr;
DesignerSettings* ApplicationCore::s_designerSettings = nullptr;
CodeEditorSettings* ApplicationCore::s_codeEditorSettings = nullptr;
ModeManager* ApplicationCore::s_modeManager = nullptr;
ServerManager* ApplicationCore::s_serverManager = nullptr;
RegistrationApiManager* ApplicationCore::s_accountManager = nullptr;
UserManager* ApplicationCore::s_userManager = nullptr;
ControlRenderingManager* ApplicationCore::s_controlRenderingManager = nullptr;
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
    QApplication::setApplicationDisplayName(APP_NAME + QStringLiteral(" (Beta)"));
    QApplication::setWindowIcon(QIcon(":/images/icon.png"));

    const QString fontPath = ":/fonts";

    /* Load default fonts */
    for (const QString& fontName : QDir(fontPath).entryList(QDir::Files))
        QFontDatabase::addApplicationFont(fontPath + '/' + fontName);

    /* Prepare setting instances */
    s_settings = new QSettings(settingsPath(), QSettings::IniFormat, app);
    s_generalSettings = new GeneralSettings(app);
    s_designerSettings = new DesignerSettings(app);
    s_codeEditorSettings = new CodeEditorSettings(app);

    /* Read settings */
    GeneralSettings::read();
    DesignerSettings::read();
    CodeEditorSettings::read();

    /* Set application's default palette */
    QApplication::setPalette(palette());
    QObject::connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged, [=]{
        QApplication::setPalette(palette());
    });

    /* Set application ui settings */
    QApplication::setFont(GeneralSettings::interfaceSettings()->toFont());
    QApplication::setStyle(new ApplicationStyle); // Ownership taken by QApplication
    QApplication::setStartDragDistance(8);

    /* Show splash screen */
    QPixmap pixmap(":/images/app/splash.png");
    pixmap.setDevicePixelRatio(QApplication::primaryScreen()->devicePixelRatio());
    pixmap = pixmap.scaled(int(512 * QApplication::primaryScreen()->devicePixelRatio()),
                            int(280 * QApplication::primaryScreen()->devicePixelRatio()),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QSplashScreen* splash(new QSplashScreen(pixmap));
    splash->show();
    app->processEvents();

    s_modeManager = new ModeManager(app);
    s_serverManager = new ServerManager(QUrl(APP_WSSSERVER), app);
    s_accountManager = new RegistrationApiManager(app);
    s_userManager = new UserManager(app);
    s_controlRenderingManager = new ControlRenderingManager(app);
    s_saveManager = new SaveManager(app);
    s_projectManager = new ProjectManager(app);
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

    DesignerScene* scene = s_windowManager->mainWindow()->centralWidget()->designerPane()->designerView()->scene();
    s_projectExposingManager->init(scene);
    s_controlCreationManager->init(scene);
    s_controlRemovingManager->init(scene);
    s_controlRenderingManager->init(scene);
    s_controlRenderingManager->scheduleDevicePixelRatioUpdate(QApplication::primaryScreen()->devicePixelRatio());

    QObject::connect(s_documentManager, &DocumentManager::initialized, [=] {
        s_windowManager->welcomeWindow()->show();
        splash->finish(s_windowManager->welcomeWindow());
        QTimer::singleShot(2000, [=] { delete splash; });
    });
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

void ApplicationCore::prepare()
{
    // Set those here, needed by QStandardPaths
    QApplication::setApplicationName(APP_NAME);
    QApplication::setOrganizationName(APP_CORP);
    QApplication::setApplicationVersion(APP_VER);
    QApplication::setOrganizationDomain(APP_DOMAIN);

    QSettings settings(settingsPath(), QSettings::IniFormat);
    if (settings.value("General/Interface.HdpiEnabled", InterfaceSettings(0).hdpiEnabled).toBool()) {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules("qtc*.info=false\n"
                                     "qtc*.debug=false\n"
                                     "qtc*.warning=false\n"
                                     "qtc*.critical=false\n"
                                     "qtc*=false");
}

void ApplicationCore::run(QApplication* app)
{
    static ApplicationCore instance(app);
    Q_UNUSED(instance)
}

QPalette ApplicationCore::palette()
{
    //  FIXME  QSettings settings(settingsPath(), QSettings::IniFormat);
    //    if (settings.value("General/Interface.Theme", InterfaceSettings().theme).toString() == "Light")
    QPalette palette(QApplication::palette());
    palette.setColor(QPalette::Active, QPalette::Text, "#2f2f2f");
    palette.setColor(QPalette::Inactive, QPalette::Text, "#2f2f2f");
    palette.setColor(QPalette::Disabled, QPalette::Text, "#6f7e8c");
    palette.setColor(QPalette::Active, QPalette::WindowText, "#2f2f2f");
    palette.setColor(QPalette::Inactive, QPalette::WindowText, "#2f2f2f");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, "#6f7e8c");
    palette.setColor(QPalette::Active, QPalette::ButtonText, "#2f2f2f");
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, "#2f2f2f");
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#6f7e8c");
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, "#bfbfbf");
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, "#bfbfbf");
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, "#bfbfbf");
    palette.setColor(QPalette::Base, "#ffffff");
    palette.setColor(QPalette::AlternateBase, "#f5f5f5");
    palette.setColor(QPalette::Button, "#ececec");
    palette.setColor(QPalette::Window, "#ececec");
    palette.setColor(QPalette::Highlight, GeneralSettings::interfaceSettings()->highlightColor);
    palette.setColor(QPalette::BrightText, "#ffffff");
    palette.setColor(QPalette::HighlightedText, "#ffffff");
    palette.setColor(QPalette::ToolTipText, "#2f2f2f");
    palette.setColor(QPalette::ToolTipBase, "#ececec");
    palette.setColor(QPalette::Link, "#025dbf");
    palette.setColor(QPalette::LinkVisited, "#B44B46");
    return palette;
}

QSettings* ApplicationCore::settings()
{
    return s_settings;
}

QString ApplicationCore::modulesPath()
{
    // TODO : Think about unix and windows versions too
    return QFileInfo(QApplication::applicationDirPath() + "/../Frameworks/modules").canonicalFilePath();
}

QString ApplicationCore::settingsPath()
{
    return appDataLocation() + "/Settings.ini";
}

QString ApplicationCore::resourcePath()
{
    return ":";
}

QString ApplicationCore::documentsPath()
{
#if defined(Q_OS_MACOS)
    return QFileInfo(QApplication::applicationDirPath() + "/../Resources/Documents").canonicalFilePath();
#else
    return QFileInfo(QApplication::applicationDirPath() + "/Documents").canonicalFilePath();
#endif
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
    WindowManager::mainWindow()->charge();
}

void ApplicationCore::onUserSessionStop()
{
    ProjectManager::stop();
}

void ApplicationCore::onUserSessionStart()
{
    // DocumentManager::load();
}