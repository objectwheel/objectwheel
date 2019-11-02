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
#include <QtWebEngine>
#include <QApplication>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

ApplicationCore::ApplicationCore(QObject* parent) : QObject(parent)
{
    DesignerSupport::activateDesignerWindowManager();
    DesignerSupport::activateDesignerMode();
    RenderUtils::stopUnifiedTimer();
    qRegisterMetaType<RendererCommands>("RendererCommands");

    m_renderSocket = new RenderSocket;
    m_socketThread = new QThread(this);

    Components::init();

    m_renderSocket->moveToThread(m_socketThread);
    m_socketThread->start();

    m_commandDispatcher = new CommandDispatcher(m_renderSocket, this);
    m_renderEngine = new RenderEngine(this);

    connect(m_renderSocket, &RenderSocket::disconnected,
            std::bind(&ApplicationCore::startQuitCountdown, this, 1200));
    connect(m_commandDispatcher, &CommandDispatcher::init,
            m_renderEngine, &RenderEngine::init);
    connect(m_commandDispatcher, &CommandDispatcher::bindingUpdate,
            m_renderEngine, &RenderEngine::updateBinding);
    connect(m_commandDispatcher, &CommandDispatcher::propertyUpdate,
            m_renderEngine, &RenderEngine::updateProperty);
    connect(m_commandDispatcher, &CommandDispatcher::controlCreation,
            m_renderEngine, &RenderEngine::createControl);
    connect(m_commandDispatcher, &CommandDispatcher::preview,
            m_renderEngine, &RenderEngine::preview);
    connect(m_commandDispatcher, &CommandDispatcher::refresh,
            m_renderEngine, &RenderEngine::refresh);
    connect(m_commandDispatcher, &CommandDispatcher::parentUpdate,
            m_renderEngine, &RenderEngine::updateParent);
    connect(m_commandDispatcher, &CommandDispatcher::indexUpdate,
            m_renderEngine, &RenderEngine::updateIndex);
    connect(m_commandDispatcher, &CommandDispatcher::idUpdate,
            m_renderEngine, &RenderEngine::updateId);
    connect(m_commandDispatcher, &CommandDispatcher::controlDeletion,
            m_renderEngine, &RenderEngine::deleteControl);
    connect(m_commandDispatcher, &CommandDispatcher::formDeletion,
            m_renderEngine, &RenderEngine::deleteForm);
    connect(m_commandDispatcher, &CommandDispatcher::formCreation,
            m_renderEngine, &RenderEngine::createForm);
    connect(m_commandDispatcher, &CommandDispatcher::controlCodeUpdate,
            m_renderEngine, &RenderEngine::updateControlCode);
    connect(m_commandDispatcher, &CommandDispatcher::formCodeUpdate,
            m_renderEngine, &RenderEngine::updateFormCode);
    connect(m_commandDispatcher, &CommandDispatcher::devicePixelRatioUpdate,
            m_renderEngine, &RenderEngine::setDevicePixelRatio);
    connect(m_renderEngine, &RenderEngine::initializationProgressChanged,
            m_commandDispatcher, &CommandDispatcher::scheduleInitializationProgress);
    connect(m_renderEngine, &RenderEngine::renderDone,
            m_commandDispatcher, &CommandDispatcher::scheduleRenderDone);
    connect(m_renderEngine, &RenderEngine::previewDone,
            m_commandDispatcher, &CommandDispatcher::schedulePreviewDone);
}

ApplicationCore::~ApplicationCore()
{    
    m_socketThread->quit();
    m_socketThread->wait();
    delete m_renderSocket;
}

void ApplicationCore::run()
{
    // Caution! Receiver is living on another thread
    QMetaObject::invokeMethod(m_renderSocket, "start", Qt::QueuedConnection,
                              Q_ARG(QString, CommandlineParser::serverName()));

    startQuitCountdown(30000);
}

void ApplicationCore::prepare()
{
    /* Prioritize down */
#if defined(Q_OS_UNIX)
    nice(19);
#elif defined(Q_OS_WIN)
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif

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
    // Not needed on desktop platforms since it
    // is already called by QtWebView::initialize()
    // QtWebEngine::initialize();
    // Also we are calling following before
    // Constructing the QApplication because
    // It uses QtWebEngine as the backend on
    // desktop platforms and it must be initialized
    // before the QApplication constructor
    QtWebView::initialize();
}

void ApplicationCore::startQuitCountdown(int msec)
{
    QTimer::singleShot(msec, this, &ApplicationCore::quitIfDisconnected);
}

void ApplicationCore::quitIfDisconnected()
{
    if (m_renderSocket->state() != QLocalSocket::ConnectedState) {
        qWarning() << tr("No connection, quitting...");
        QMetaObject::invokeMethod(qApp, [] { qApp->exit(EXIT_FAILURE); }, Qt::QueuedConnection);
        qApp->exit(EXIT_FAILURE);
    }
}
