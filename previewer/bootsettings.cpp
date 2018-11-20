#include <bootsettings.h>
#include <commandlineparser.h>
#include <quicktheme.h>

#include <QApplication>

void BootSettings::apply()
{
    QuickTheme::setTheme(CommandlineParser::projectDirectory());

    // Boot settings
    qputenv("QML_DISABLE_DISK_CACHE", "true");
    qputenv("QSG_DISTANCEFIELD_ANTIALIASING", "gray");

#ifdef Q_OS_MACOS // Disable focus stealing on macOS
    qputenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM", "true");
#endif

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
}