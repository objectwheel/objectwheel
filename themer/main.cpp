#include <delayer.h>
#include <saveutils.h>
#include <quicktheme.h>
#include <utilityfunctions.h>
#include <signalwatcher.h>
#include <appconstants.h>

#ifdef Q_OS_MACOS
#  include <macoperations.h>
#endif

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    qputenv("QML_DISABLE_DISK_CACHE", "true");

#ifdef Q_OS_MACOS // Disable focus stealing on macOS
    if (argc > 1 && argv[1] == QString("capture"))
        qputenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM", "true");
#endif

    int version = 0;
    if (argc > 2)
        QuickTheme::setTheme(argv[2], &version);

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Initialize application
    QApplication a(argc, argv);
    QApplication::setApplicationName(AppConstants::NAME);
    QApplication::setOrganizationName(AppConstants::COMPANY);
    QApplication::setApplicationVersion(AppConstants::VERSION);
    QApplication::setOrganizationDomain(AppConstants::DOMAIN);
    QApplication::setApplicationDisplayName(AppConstants::LABEL);
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));
    QApplication::setFont(UtilityFunctions::systemDefaultFont());

    // Handle signals
    QObject::connect(SignalWatcher::instance(), &SignalWatcher::signal,
                     SignalWatcher::instance(), &SignalWatcher::defaultInterruptAction,
                     Qt::QueuedConnection);

#ifdef Q_OS_MACOS // Show/hide dock icon
    if (argc > 1 && argv[1] == QString("capture"))
        MacOperations::setDockIconVisible(false);
    else
        MacOperations::setDockIconVisible(true);
#endif

    QQmlApplicationEngine engine;

    if (version > 0) {
        if (version == 1) // v1 - v2
            engine.load(QUrl(QStringLiteral("qrc:/theme.qml")));
        else
            engine.load(QUrl(QStringLiteral("qrc:/theme2.qml")));

        if (engine.rootObjects().isEmpty())
            return EXIT_FAILURE;

        auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());

        if (argc > 3 && argv[1] == QString("capture")) { // capture - show
            window->setFlags(
                Qt::Tool |
                Qt::FramelessWindowHint |
                Qt::WindowStaysOnBottomHint |
                Qt::WindowDoesNotAcceptFocus
            );
            window->create();
            window->update();
            Delayer::delay(300);
            window->grabWindow().save(argv[3], "PNG");
            return EXIT_SUCCESS;
        } else {
            window->setFlags(
                window->flags() |
                Qt::WindowStaysOnTopHint
            );
            window->show();
            return a.exec();
        }
    }

    return EXIT_FAILURE;
}