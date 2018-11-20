#include <delayer.h>
#include <filemanager.h>
#include <saveutils.h>
#include <quicktheme.h>

#include <QtQml>
#include <QtQuick>
#include <QtWidgets>

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
    QApplication::setOrganizationName("Objectwheel");
    QApplication::setOrganizationDomain("objectwheel.com");
    QApplication::setApplicationName("themer");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setWindowIcon(QIcon(":/images/owicon.png"));

    /* Load default fonts */
    const QString fontPath = ":/fonts";
    for (const QString& fontName : lsfile(fontPath))
        QFontDatabase::addApplicationFont(fontPath + separator() + fontName);

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