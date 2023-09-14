#include <delayer.h>
#include <saveutils.h>
#include <quicktheme.h>
#include <utilityfunctions.h>
#include <signalhandler.h>
#include <appconstants.h>

#ifdef Q_OS_MACOS
#  include <macoperations.h>
#endif

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    qputenv("QT_FORCE_STDERR_LOGGING", "1");
    qputenv("QML_DISABLE_DISK_CACHE", "true");

    if (argc > 1 && QString::fromUtf8(argv[1]).toInt()) {
//        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
//        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    }

#ifdef Q_OS_MACOS // Disable focus stealing on macOS
    if (argc > 2 && argv[2] == QString("capture"))
        qputenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM", "true");
#endif

    int version = 0;
    if (argc > 3)
        QuickTheme::setTheme(argv[3], &version);

    // Initialize application
    QApplication a(argc, argv);
    QApplication::setApplicationName(AppConstants::NAME);
    QApplication::setOrganizationName(AppConstants::COMPANY);
    QApplication::setApplicationVersion(AppConstants::VERSION);
    QApplication::setOrganizationDomain(AppConstants::ROOT_DOMAIN);
    QApplication::setApplicationDisplayName(AppConstants::LABEL);
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));
    QApplication::setFont(UtilityFunctions::systemDefaultFont());

    // Handle signals
    SignalHandler signalHandler;
    QObject::connect(&signalHandler, &SignalHandler::interrupted,
                     &signalHandler, &SignalHandler::exitGracefully);

#ifdef Q_OS_MACOS // Show/hide dock icon
    if (argc > 2 && argv[2] == QString("capture"))
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
        Q_ASSERT(window);

        if (argc > 4 && argv[2] == QString("capture")) { // capture - show
            window->setFlags(
                Qt::Tool |
                Qt::FramelessWindowHint |
                Qt::WindowStaysOnBottomHint |
                Qt::WindowDoesNotAcceptFocus
            );
            window->create();
            window->update();
            Delayer::delay(300);
            window->grabWindow().save(argv[4], "PNG");
            return EXIT_SUCCESS;
        } else {
            window->show();
            window->requestActivate();
            return a.exec();
        }
    }

    return EXIT_FAILURE;
}
