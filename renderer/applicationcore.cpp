#include <applicationcore.h>
#include <rendersocket.h>
#include <renderengine.h>
#include <renderutils.h>
#include <commandlineparser.h>
#include <commanddispatcher.h>
#include <quicktheme.h>
#include <saveutils.h>
#include <utilityfunctions.h>

#include <private/qquickdesignersupport_p.h>

#include <QTimer>
#include <QApplication>
#include <QDir>
#include <QThread>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

ApplicationCore::ApplicationCore(QObject* parent) : QObject(parent)
{
    /** Core initialization **/
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName(QStringLiteral(APP_NAME));
    QApplication::setOrganizationName(QStringLiteral(APP_CORP));
    QApplication::setApplicationVersion(QStringLiteral(APP_VER));
    QApplication::setOrganizationDomain(QStringLiteral(APP_DOMAIN));
    QApplication::setApplicationDisplayName(QStringLiteral(APP_NAME) + QObject::tr(" Renderer"));

    /* Set application ui settings */
    QApplication::setFont(UtilityFunctions::systemDefaultFont());
    QApplication::setStartDragDistance(8);

    DesignerSupport::activateDesignerWindowManager();
    DesignerSupport::activateDesignerMode();
    RenderUtils::stopUnifiedTimer();
    qRegisterMetaType<RendererCommands>("RendererCommands");

    m_renderSocket = new RenderSocket;
    m_socketThread = new QThread(this);

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

QString ApplicationCore::modulesPath()
{
    // TODO : Think about unix and windows versions too
    return QFileInfo("../Frameworks/Modules").canonicalFilePath();
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
