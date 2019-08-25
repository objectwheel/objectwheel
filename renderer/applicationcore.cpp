#include <applicationcore.h>
#include <rendersocket.h>
#include <components.h>
#include <renderengine.h>
#include <renderutils.h>
#include <commandlineparser.h>
#include <commanddispatcher.h>
#include <quicktheme.h>
#include <saveutils.h>

#include <private/qquickdesignersupport_p.h>

#include <QTimer>
#include <QtWebView>
#include <QApplication>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

RenderSocket* ApplicationCore::s_renderSocket = nullptr;
QThread* ApplicationCore::s_socketThread = nullptr;
CommandDispatcher* ApplicationCore::s_commandDispatcher = nullptr;
RenderEngine* ApplicationCore::s_renderEngine = nullptr;

ApplicationCore::ApplicationCore(QObject* parent) : QObject(parent)
{
    /* Prioritize down */
#if defined(Q_OS_UNIX)
    nice(19);
#elif defined(Q_OS_WIN)
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif

    DesignerSupport::activateDesignerWindowManager();
    DesignerSupport::activateDesignerMode();
    RenderUtils::stopUnifiedTimer();
    QtWebView::initialize();
    qRegisterMetaType<RendererCommands>("RendererCommands");

    s_renderSocket = new RenderSocket;
    s_socketThread = new QThread(this);

    Components::init();

    s_renderSocket->moveToThread(s_socketThread);
    s_socketThread->start();

    s_commandDispatcher = new CommandDispatcher(s_renderSocket, this);
    s_renderEngine = new RenderEngine(this);

    connect(s_renderSocket, &RenderSocket::disconnected,
            std::bind(&ApplicationCore::startQuitCountdown, this, 1200));

    connect(s_commandDispatcher, &CommandDispatcher::terminate,
            this, &ApplicationCore::onTerminateCommand);
    connect(s_commandDispatcher, &CommandDispatcher::init,
            s_renderEngine, &RenderEngine::init);
    connect(s_commandDispatcher, &CommandDispatcher::bindingUpdate,
            s_renderEngine, &RenderEngine::updateBinding);
    connect(s_commandDispatcher, &CommandDispatcher::propertyUpdate,
            s_renderEngine, &RenderEngine::updateProperty);
    connect(s_commandDispatcher, &CommandDispatcher::controlCreation,
            s_renderEngine, &RenderEngine::createControl);
    connect(s_commandDispatcher, &CommandDispatcher::preview,
            s_renderEngine, &RenderEngine::preview);
    connect(s_commandDispatcher, &CommandDispatcher::refresh,
            s_renderEngine, &RenderEngine::refresh);
    connect(s_commandDispatcher, &CommandDispatcher::parentUpdate,
            s_renderEngine, &RenderEngine::updateParent);
    connect(s_commandDispatcher, &CommandDispatcher::indexUpdate,
            s_renderEngine, &RenderEngine::updateIndex);
    connect(s_commandDispatcher, &CommandDispatcher::idUpdate,
            s_renderEngine, &RenderEngine::updateId);
    connect(s_commandDispatcher, &CommandDispatcher::controlDeletion,
            s_renderEngine, &RenderEngine::deleteControl);
    connect(s_commandDispatcher, &CommandDispatcher::formDeletion,
            s_renderEngine, &RenderEngine::deleteForm);
    connect(s_commandDispatcher, &CommandDispatcher::formCreation,
            s_renderEngine, &RenderEngine::createForm);
    connect(s_commandDispatcher, &CommandDispatcher::controlCodeUpdate,
            s_renderEngine, &RenderEngine::updateControlCode);
    connect(s_commandDispatcher, &CommandDispatcher::formCodeUpdate,
            s_renderEngine, &RenderEngine::updateFormCode);
    connect(s_commandDispatcher, &CommandDispatcher::devicePixelRatioUpdate,
            s_renderEngine, &RenderEngine::setDevicePixelRatio);

    connect(s_renderEngine, &RenderEngine::initializationProgressChanged,
            s_commandDispatcher, &CommandDispatcher::scheduleInitializationProgress);
    connect(s_renderEngine, &RenderEngine::renderDone,
            s_commandDispatcher, &CommandDispatcher::scheduleRenderDone);
    connect(s_renderEngine, &RenderEngine::previewDone,
            s_commandDispatcher, &CommandDispatcher::schedulePreviewDone);

    QMetaObject::invokeMethod(s_renderSocket, "start",
                              Q_ARG(QString, CommandlineParser::serverName()));

    startQuitCountdown(30000);
}

ApplicationCore::~ApplicationCore()
{
    QMetaObject::invokeMethod(s_renderSocket, "deleteLater");

    s_socketThread->quit();
    s_socketThread->wait();
}

void ApplicationCore::init(QObject* parent)
{
    static ApplicationCore* instance = nullptr;
    if (instance)
        return;

    instance = new ApplicationCore(parent);
}

void ApplicationCore::prepare()
{
    QApplication::setOrganizationName("Objectwheel");
    QApplication::setOrganizationDomain("objectwheel.com");
    QApplication::setApplicationName("renderer");
    QApplication::setApplicationVersion("1.1.0");
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QuickTheme::setTheme(CommandlineParser::projectDirectory());

    // Since we always render text into an FBO, we need to globally disable
    // subpixel antialiasing and instead use gray.
    qputenv("QSG_DISTANCEFIELD_ANTIALIASING", "gray");
#ifdef Q_OS_OSX //This keeps qml2puppet from stealing focus
    qputenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM", "true");
#endif
    qputenv("QML_BAD_GUI_RENDER_LOOP", "true");
    qputenv("QML_PUPPET_MODE", "true");
    qputenv("QML_DISABLE_DISK_CACHE", "true");
    qputenv("QT_QUICK_CONTROLS_CONF",
            SaveUtils::toProjectAssetsDir(CommandlineParser::projectDirectory()).toUtf8());
}

void ApplicationCore::startQuitCountdown(int msec)
{
    QTimer::singleShot(msec, this, &ApplicationCore::quitIfDisconnected);
}

void ApplicationCore::onTerminateCommand()
{
    QMetaObject::invokeMethod(s_renderSocket, "abort");
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit");
}

void ApplicationCore::quitIfDisconnected()
{
    if (s_renderSocket->state() != QLocalSocket::ConnectedState) {
        qWarning() << tr("No connection, quitting...");
        QCoreApplication::exit(EXIT_FAILURE);
    }
}
