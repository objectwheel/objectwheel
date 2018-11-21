#include <applicationcore.h>
#include <previewersocket.h>
#include <components.h>
#include <previewer.h>
#include <previewerutils.h>
#include <commandlineparser.h>
#include <commanddispatcher.h>
#include <filemanager.h>

#include <private/qquickdesignersupport_p.h>

#include <QTimer>
#include <QtWebView>
#include <QApplication>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

PreviewerSocket* ApplicationCore::s_previewerSocket = nullptr;
QThread* ApplicationCore::s_socketThread = nullptr;
CommandDispatcher* ApplicationCore::s_commandDispatcher = nullptr;
Previewer* ApplicationCore::s_previewer = nullptr;

ApplicationCore::ApplicationCore(QObject* parent) : QObject(parent)
{
    QApplication::setOrganizationName("Objectwheel");
    QApplication::setOrganizationDomain("objectwheel.com");
    QApplication::setApplicationName("previewer");
    QApplication::setApplicationVersion("1.1.0");
    QApplication::setQuitOnLastWindowClosed(false);

    /* Prioritize down */
#if defined(Q_OS_UNIX)
    nice(19);
#elif defined(Q_OS_WIN)
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif

    /* Load default fonts */
    const QString fontPath = ":/fonts";
    for (const QString& fontName : lsfile(fontPath))
        QFontDatabase::addApplicationFont(fontPath + separator() + fontName);

    DesignerSupport::activateDesignerWindowManager();
    DesignerSupport::activateDesignerMode();
    PreviewerUtils::stopUnifiedTimer();
    Components::init();
    QtWebView::initialize();
    qRegisterMetaType<PreviewerCommands>("PreviewerCommands");

    s_previewerSocket = new PreviewerSocket;
    s_socketThread = new QThread(this);

    s_previewerSocket->moveToThread(s_socketThread);
    s_socketThread->start();

    s_commandDispatcher = new CommandDispatcher(s_previewerSocket, this);
    s_previewer = new Previewer(this);

    connect(s_previewerSocket, &PreviewerSocket::disconnected,
            std::bind(&ApplicationCore::startQuitCountdown, this, 1200));

    connect(s_commandDispatcher, &CommandDispatcher::terminate,
            this, &ApplicationCore::onTerminateCommand);
    connect(s_commandDispatcher, &CommandDispatcher::init,
            s_previewer, &Previewer::init);
    connect(s_commandDispatcher, &CommandDispatcher::propertyUpdate,
            s_previewer, &Previewer::updateProperty);
    connect(s_commandDispatcher, &CommandDispatcher::controlCreation,
            s_previewer, &Previewer::createControl);
    connect(s_commandDispatcher, &CommandDispatcher::refresh,
            s_previewer, &Previewer::refresh);
    connect(s_commandDispatcher, &CommandDispatcher::parentUpdate,
            s_previewer, &Previewer::updateParent);
    connect(s_commandDispatcher, &CommandDispatcher::idUpdate,
            s_previewer, &Previewer::updateId);
    connect(s_commandDispatcher, &CommandDispatcher::controlDeletion,
            s_previewer, &Previewer::deleteControl);
    connect(s_commandDispatcher, &CommandDispatcher::formDeletion,
            s_previewer, &Previewer::deleteForm);
    connect(s_commandDispatcher, &CommandDispatcher::formCreation,
            s_previewer, &Previewer::createForm);
    connect(s_commandDispatcher, &CommandDispatcher::controlCodeUpdate,
            s_previewer, &Previewer::updateControlCode);
    connect(s_commandDispatcher, &CommandDispatcher::formCodeUpdate,
            s_previewer, &Previewer::updateFormCode);

    connect(s_previewer, &Previewer::initializationProgressChanged,
            s_commandDispatcher, &CommandDispatcher::scheduleInitializationProgress);
    connect(s_previewer, &Previewer::previewDone,
            s_commandDispatcher, &CommandDispatcher::schedulePreviewDone);

    QMetaObject::invokeMethod(s_previewerSocket, "start",
                              Q_ARG(QString, CommandlineParser::serverName()));

    startQuitCountdown(30000);
}

ApplicationCore::~ApplicationCore()
{
    QMetaObject::invokeMethod(s_previewerSocket, "deleteLater");

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

void ApplicationCore::startQuitCountdown(int msec)
{
    QTimer::singleShot(msec, this, &ApplicationCore::quitIfDisconnected);
}

void ApplicationCore::onTerminateCommand()
{
    QMetaObject::invokeMethod(s_previewerSocket, "abort");
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit");
}

void ApplicationCore::quitIfDisconnected()
{
    if (s_previewerSocket->state() != QLocalSocket::ConnectedState) {
        qWarning() << tr("No connection, quitting...");
        QCoreApplication::exit(EXIT_FAILURE);
    }
}