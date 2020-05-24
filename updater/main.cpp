#include <QCoreApplication>

int main(int argc, char *argv[])
{
#ifdef Q_OS_MACOS // Disable focus stealing on macOS
    if (argc > 1 && argv[1] == QString("capture"))
        qputenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM", "true");
#endif

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Initialize application
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral(APP_NAME));
    QCoreApplication::setOrganizationName(QStringLiteral(APP_CORP));
    QCoreApplication::setApplicationVersion(QStringLiteral(APP_VER));
    QCoreApplication::setOrganizationDomain(QStringLiteral(APP_DOMAIN));

    return EXIT_FAILURE;
}