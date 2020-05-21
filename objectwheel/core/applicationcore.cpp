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
#include <systemsettings.h>
#include <applicationstyle.h>
#include <helpmanager.h>
#include <paintutils.h>
#include <servermanager.h>
#include <modemanager.h>
#include <splashscreen.h>
#include <signalwatcher.h>
#include <updatemanager.h>

#include <QToolTip>
#include <QScreen>
#include <QStandardPaths>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QFontDatabase>
#include <QSharedMemory>
#include <QTimer>

#include <theme/theme_p.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/themechooser.h>

#if defined(Q_OS_MACOS)
#  include <macoperations.h>
#endif

QSettings* ApplicationCore::s_settings = nullptr;
GeneralSettings* ApplicationCore::s_generalSettings = nullptr;
DesignerSettings* ApplicationCore::s_designerSettings = nullptr;
CodeEditorSettings* ApplicationCore::s_codeEditorSettings = nullptr;
SystemSettings* ApplicationCore::s_systemSettings = nullptr;
ModeManager* ApplicationCore::s_modeManager = nullptr;
ServerManager* ApplicationCore::s_serverManager = nullptr;
UpdateManager* ApplicationCore::s_updateManager = nullptr;
RegistrationApiManager* ApplicationCore::s_registrationApiManager = nullptr;
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

ApplicationCore::ApplicationCore()
{
    /** Core initialization **/
    QApplication::setApplicationDisplayName(QStringLiteral(APP_NAME) + QStringLiteral(" (Beta)"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));

    // Handle signals
    QObject::connect(SignalWatcher::instance(), &SignalWatcher::signal,
                     SignalWatcher::instance(), &SignalWatcher::defaultInterruptAction,
                     Qt::QueuedConnection);

    /* Load default fonts */
    for (const QString& fontName : QDir(QStringLiteral(":/fonts")).entryList(QDir::Files))
        QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/") + fontName);

    /* Prepare setting instances */
    s_settings = new QSettings(settingsPath(), QSettings::IniFormat);
    s_generalSettings = new GeneralSettings;
    s_designerSettings = new DesignerSettings;
    s_codeEditorSettings = new CodeEditorSettings;
    s_systemSettings = new SystemSettings;

    /* Read settings */
    GeneralSettings::read();
    DesignerSettings::read();
    CodeEditorSettings::read();
    SystemSettings::read();

    /* Set application's default palette */
    QApplication::setPalette(palette());
    QObject::connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged, [=]{
        QApplication::setPalette(palette());
    });

    /* Set application ui settings */
    QFont font = GeneralSettings::interfaceSettings()->toFont();
    QApplication::setStyle(new ApplicationStyle); // Ownership taken by QApplication
    QApplication::setFont(font); // Call after style change github.com/qt/qtbase/commit/14071b5
    QApplication::setStartDragDistance(8);
    font.setPixelSize(font.pixelSize() - 1);
    QToolTip::setFont(font);

    /* Show splash screen */
    auto splash = new SplashScreen(PaintUtils::pixmap(QIcon(":/images/splash/splash.png"),
                                                      QSize(485, 300)), Qt::WindowStaysOnTopHint);
    splash->showMessage(QObject::tr("Initializing..."));
    splash->show();

    // App Nap feature of macOS prevents timers to accurately emit signals at the right time
    // when the application minimized or working in the background. That especially affects
    // the disconnection detection of the ServerManager
#if defined(Q_OS_MACOS)
    MacOperations::disableAppNap(QLatin1String("Listening to server"));
#endif

    s_modeManager = new ModeManager;
    s_serverManager = new ServerManager(QUrl(QStringLiteral(APP_WSSSERVER)));
    s_updateManager = new UpdateManager;
    s_registrationApiManager = new RegistrationApiManager;
    s_userManager = new UserManager;
    s_controlRenderingManager = new ControlRenderingManager;
    s_saveManager = new SaveManager;
    s_projectManager = new ProjectManager;
    s_projectExposingManager = new ProjectExposingManager;
    s_controlCreationManager = new ControlCreationManager;
    s_controlRemovingManager = new ControlRemovingManager;
    s_controlPropertyManager = new ControlPropertyManager;
    s_runManager = new RunManager;
    s_helpManager = new HelpManager;

    s_helpManager->setupHelpManager();
    Utils::setCreatorTheme(Core::Internal::ThemeEntry::createTheme(Core::Constants::DEFAULT_THEME));
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     s_helpManager, &HelpManager::aboutToShutdown);

    s_documentManager = new DocumentManager;

    /** Ui initialization **/
    s_windowManager = new WindowManager;
    s_menuManager = new MenuManager;

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

    auto hideSplashScreen = [=] {
        s_windowManager->welcomeWindow()->show();
        splash->finish(s_windowManager->welcomeWindow());
        QTimer::singleShot(2000, [=] { delete splash; });
    };
    if (s_documentManager->isInitialized())
        hideSplashScreen();
    else
        QObject::connect(s_documentManager, &DocumentManager::initialized, hideSplashScreen);
}

ApplicationCore::~ApplicationCore()
{
    delete s_menuManager;
    s_menuManager = nullptr;
    delete s_windowManager;
    s_windowManager = nullptr;
    delete s_documentManager;
    s_documentManager = nullptr;
    delete s_helpManager;
    s_helpManager = nullptr;
    delete s_runManager;
    s_runManager = nullptr;
    delete s_controlPropertyManager;
    s_controlPropertyManager = nullptr;
    delete s_controlRemovingManager;
    s_controlRemovingManager = nullptr;
    delete s_controlCreationManager;
    s_controlCreationManager = nullptr;
    delete s_projectExposingManager;
    s_projectExposingManager = nullptr;
    delete s_projectManager;
    s_projectManager = nullptr;
    delete s_saveManager;
    s_saveManager = nullptr;
    delete s_controlRenderingManager;
    s_controlRenderingManager = nullptr;
    delete s_userManager;
    s_userManager = nullptr;
    delete s_registrationApiManager;
    s_registrationApiManager = nullptr;
    delete s_updateManager;
    s_updateManager = nullptr;
    delete s_serverManager;
    s_serverManager = nullptr;
    delete s_modeManager;
    s_modeManager = nullptr;
    delete s_systemSettings;
    s_systemSettings = nullptr;
    delete s_codeEditorSettings;
    s_codeEditorSettings = nullptr;
    delete s_designerSettings;
    s_designerSettings = nullptr;
    delete s_generalSettings;
    s_generalSettings = nullptr;
    delete s_settings;
    s_settings = nullptr;
}

bool ApplicationCore::locked()
{
    QSharedMemory* sharedMemory = new QSharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory->create(1)) {
        sharedMemory->attach();
        sharedMemory->detach();
        if(!sharedMemory->create(1)) {
            QApplication::setStyle(new ApplicationStyle); // Ownership taken by QApplication
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
    QApplication::setApplicationName(QStringLiteral(APP_NAME));
    QApplication::setOrganizationName(QStringLiteral(APP_CORP));
    QApplication::setApplicationVersion(QStringLiteral(APP_VER));
    QApplication::setOrganizationDomain(QStringLiteral(APP_DOMAIN));

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

QPalette ApplicationCore::palette()
{
    //  FIXME  QSettings settings(settingsPath(), QSettings::IniFormat);
    //    if (settings.value("General/Interface.Theme", InterfaceSettings().theme).toString() == "Light")
    const int g = qGray(GeneralSettings::interfaceSettings()->highlightColor.rgb());
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
    palette.setColor(QPalette::Active, QPalette::Highlight, GeneralSettings::interfaceSettings()->highlightColor);
    palette.setColor(QPalette::Inactive, QPalette::Highlight, GeneralSettings::interfaceSettings()->highlightColor);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(g, g, g));
    palette.setColor(QPalette::Base, "#ffffff");
    palette.setColor(QPalette::AlternateBase, "#f5f5f5");
    palette.setColor(QPalette::Button, "#ececec");
    palette.setColor(QPalette::Window, "#ececec");
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
    return QFileInfo(QCoreApplication::applicationDirPath() + QStringLiteral("/../Resources/Modules")).canonicalFilePath();
}

QString ApplicationCore::settingsPath()
{
    return appDataPath() + QLatin1String("/Settings.ini");
}

QString ApplicationCore::resourcePath()
{
    return ":";
}

QString ApplicationCore::documentsPath()
{
#if defined(Q_OS_MACOS)
    return QFileInfo(QCoreApplication::applicationDirPath() + QStringLiteral("/../Resources/Documents")).canonicalFilePath();
#else
    return QFileInfo("Documents").canonicalFilePath();
#endif
}

QString ApplicationCore::updatesPath()
{
    return appDataPath() + QLatin1String("/Updates");
}

QString ApplicationCore::appDataPath()
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
