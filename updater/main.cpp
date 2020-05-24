#include <QApplication>
#include <QIcon>

#ifdef Q_OS_MACOS
#  include <macoperations.h>
#endif

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Initialize application
    QApplication a(argc, argv);
    QApplication::setApplicationName(QStringLiteral(APP_NAME));
    QApplication::setOrganizationName(QStringLiteral(APP_CORP));
    QApplication::setApplicationVersion(QStringLiteral(APP_VER));
    QApplication::setOrganizationDomain(QStringLiteral(APP_DOMAIN));
    QApplication::setApplicationDisplayName(QStringLiteral(APP_NAME) + QObject::tr(" Updater"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/images/icon.png")));

    return a.exec();
}