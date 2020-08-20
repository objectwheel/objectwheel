#include <applicationcore.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <projectexposingmanager.h>
#include <controlproductionmanager.h>
#include <windowmanager.h>
#include <apimanager.h>
#include <controlrenderingmanager.h>
#include <documentmanager.h>
#include <mainwindow.h>
#include <runmanager.h>
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
#include <payloadmanager.h>
#include <updatemanager.h>
#include <modemanager.h>
#include <splashscreen.h>
#include <signalwatcher.h>
#include <appconstants.h>
#include <inactivitywatcher.h>

#include <QToolTip>
#include <QScreen>
#include <QStandardPaths>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QFontDatabase>
#include <QSharedMemory>
#include <QTimer>
#include <QPixmapCache>
#include <QWindow>

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
PayloadManager* ApplicationCore::s_payloadManager = nullptr;
UpdateManager* ApplicationCore::s_updateManager = nullptr;
ApiManager* ApplicationCore::s_apiManager = nullptr;
UserManager* ApplicationCore::s_userManager = nullptr;
ControlRenderingManager* ApplicationCore::s_controlRenderingManager = nullptr;
SaveManager* ApplicationCore::s_saveManager = nullptr;
ProjectManager* ApplicationCore::s_projectManager = nullptr;
ProjectExposingManager* ApplicationCore::s_projectExposingManager = nullptr;
ControlProductionManager* ApplicationCore::s_controlProductionManager = nullptr;
ControlRemovingManager* ApplicationCore::s_controlRemovingManager = nullptr;
ControlPropertyManager* ApplicationCore::s_controlPropertyManager = nullptr;
RunManager* ApplicationCore::s_runManager = nullptr;
HelpManager* ApplicationCore::s_helpManager = nullptr;
DocumentManager* ApplicationCore::s_documentManager = nullptr;
WindowManager* ApplicationCore::s_windowManager = nullptr;
MenuManager* ApplicationCore::s_menuManager = nullptr;
InactivityWatcher* ApplicationCore::s_inactivityWatcher = nullptr;

ApplicationCore::ApplicationCore()
{
    /** Core initialization **/
    QApplication::setApplicationDisplayName(AppConstants::LABEL);
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
        QPixmapCache::clear(); // Cached QIcon pixmaps makes use of old palette, so we have to update
    });

    /* Set application ui settings */
    QFont font = GeneralSettings::interfaceSettings()->toFont();
    QApplication::setStyle(new ApplicationStyle); // Ownership taken by QApplication
    QApplication::setFont(font); // Call after style change github.com/qt/qtbase/commit/14071b5
    QApplication::setStartDragDistance(8);
    font.setPixelSize(font.pixelSize() - 1);
    QToolTip::setFont(font);

    /* Show splash screen */
    auto splash = new SplashScreen(PaintUtils::pixmap(QStringLiteral(":/images/splash/splash.png"),
                                                      QSize(485, 300)), Qt::WindowStaysOnTopHint);
    // For some reason if we use showMessage() multiple
    // times the QApplication::processEvents() is called
    // hence the application behaves weirdly.
    splash->showMessage(QObject::tr("Initializing..."));
    splash->show();

    s_modeManager = new ModeManager;
    s_serverManager = new ServerManager;
    s_payloadManager = new PayloadManager;
    s_updateManager = new UpdateManager;
    s_apiManager = new ApiManager;
    s_userManager = new UserManager;
    s_controlRenderingManager = new ControlRenderingManager;
    s_saveManager = new SaveManager;
    s_projectManager = new ProjectManager;
    s_projectExposingManager = new ProjectExposingManager;
    s_controlProductionManager = new ControlProductionManager;
    s_controlRemovingManager = new ControlRemovingManager;
    s_controlPropertyManager = new ControlPropertyManager;
    s_runManager = new RunManager;
    s_helpManager = new HelpManager;

    s_helpManager->setupHelpManager();
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     s_helpManager, &HelpManager::aboutToShutdown);

    s_documentManager = new DocumentManager;

    /** Ui initialization **/
    s_windowManager = new WindowManager;
    s_menuManager = new MenuManager;
    s_inactivityWatcher = new InactivityWatcher(180);

    QObject::connect(s_inactivityWatcher, &InactivityWatcher::activated,
                     &ApplicationCore::onActivated);
    QObject::connect(s_inactivityWatcher, &InactivityWatcher::deactivated,
                     &ApplicationCore::onDeactivated);
    QObject::connect(qApp, &QApplication::focusWindowChanged,
                     s_inactivityWatcher, &InactivityWatcher::activate);
    QObject::connect(s_payloadManager, &PayloadManager::bytesWritten,
                     s_inactivityWatcher, [] { s_inactivityWatcher->activate(); });
    QObject::connect(s_payloadManager, &PayloadManager::readyRead,
                     s_inactivityWatcher, [] { s_inactivityWatcher->activate(); });
    QObject::connect(s_updateManager, &UpdateManager::updateCheckStarted,
                     s_inactivityWatcher, [] { s_inactivityWatcher->activate(); });
    QObject::connect(s_updateManager, &UpdateManager::downloadProgress,
                     s_inactivityWatcher, [] { s_inactivityWatcher->activate(); });

    QObject::connect(s_projectManager, &ProjectManager::started,
                     &ApplicationCore::onProjectStart);
    QObject::connect(s_projectManager, &ProjectManager::stopped,
                     &ApplicationCore::onProjectStop);
    //  FIXME  QObject::connect(s_userManager, &UserManager::started,
    //                     &ApplicationCore::onUserSessionStart);
    //    QObject::connect(s_userManager, &UserManager::aboutToStop,
    //                     &ApplicationCore::onUserSessionStop);

    DesignerScene* scene = s_windowManager->mainWindow()->centralWidget()->designerPane()->designerView()->scene();
    s_projectExposingManager->init(scene);
    s_controlProductionManager->init(scene);
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
    delete s_inactivityWatcher;
    s_inactivityWatcher = nullptr;
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
    delete s_controlProductionManager;
    s_controlProductionManager = nullptr;
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
    delete s_apiManager;
    s_apiManager = nullptr;
    delete s_updateManager;
    s_updateManager = nullptr;
    delete s_payloadManager;
    s_payloadManager = nullptr;
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
    QApplication::setApplicationName(AppConstants::NAME);
    QApplication::setOrganizationName(AppConstants::COMPANY);
    QApplication::setApplicationVersion(AppConstants::VERSION);
    QApplication::setOrganizationDomain(AppConstants::ROOT_DOMAIN);

    QSettings settings(settingsPath(), QSettings::IniFormat);
    if (settings.value("General/Interface.HdpiEnabled", InterfaceSettings(0).hdpiEnabled).toBool()) {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
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
    palette.setColor(QPalette::Active, QPalette::Text, "#303030");
    palette.setColor(QPalette::Inactive, QPalette::Text, "#303030");
    palette.setColor(QPalette::Disabled, QPalette::Text, "#656565");
    palette.setColor(QPalette::Active, QPalette::WindowText, "#303030");
    palette.setColor(QPalette::Inactive, QPalette::WindowText, "#303030");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, "#656565");
    palette.setColor(QPalette::Active, QPalette::ButtonText, "#303030");
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, "#303030");
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#656565");
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
    palette.setColor(QPalette::ToolTipText, "#303030");
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
    return QFileInfo(QCoreApplication::applicationDirPath() + QLatin1String("/../Resources/Modules")).canonicalFilePath();
}

QString ApplicationCore::settingsPath()
{
    return appDataPath() + QLatin1String("/Settings.ini");
}

QString ApplicationCore::resourcePath()
{
    return QStringLiteral(":");
}

QString ApplicationCore::documentsPath()
{
#if defined(Q_OS_MACOS)
    return QFileInfo(QCoreApplication::applicationDirPath() + QLatin1String("/../Resources/Documents/docs.qhc")).canonicalFilePath();
#else
    return QFileInfo(QCoreApplication::applicationDirPath() + "/Documents/docs.qhc").canonicalFilePath();
#endif
}

QString ApplicationCore::updatesPath()
{
    return appDataPath() + QLatin1String("/Updates");
}

QString ApplicationCore::stylesPath()
{
    return appDataPath() + QLatin1String("/Styles");
}

QString ApplicationCore::resourceStylesPath()
{
    return resourcePath() + QLatin1String("/Styles");
}

QString ApplicationCore::appDataPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

void ApplicationCore::onActivated()
{
    ServerManager::wake();
#if defined(Q_OS_MACOS)
    MacOperations::disableIdleSystemSleep();
#endif
}

void ApplicationCore::onDeactivated()
{
    ServerManager::sleep();
#if defined(Q_OS_MACOS)
    MacOperations::enableIdleSystemSleep();
#endif
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
