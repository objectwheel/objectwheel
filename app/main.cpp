#include <css.h>
#include <filemanager.h>
#include <menumanager.h>
#include <windowmanager.h>
#include <initializationmanager.h>
#include <welcomewindow.h>
#include <appfontsettings.h>

#include <QIcon>
#include <QMessageBox>
#include <QApplication>
#include <QFontDatabase>
#include <QSharedMemory>
#include <QLoggingCategory>

int main(int argc, char* argv[])
{
    // Boot Settings
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL); // For reCaptcha
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules(QStringLiteral("qtc*.info=false\n"
                                                    "qtc*.debug=false\n"
                                                    "qtc*.warning=false\n"
                                                    "qtc*.critical=false\n"
                                                    "qtc*=false"));

    // Initialize application
    QApplication app(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setOrganizationName(APP_CORP);
    QApplication::setApplicationVersion(APP_VER);
    QApplication::setOrganizationDomain(APP_DOMAIN);
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setWindowIcon(QIcon(":/images/owicon.png"));

    // Multiple instances protection
    QSharedMemory sharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory.create(1)) {
        sharedMemory.attach();
        sharedMemory.detach();
        if(!sharedMemory.create(1)) {
            QMessageBox::warning(nullptr,
                                 QObject::tr("Quitting"),
                                 QObject::tr("Another instance is already running."));
            app.exit();
            return 0;
        }
    }

    /* Set Font */
    AppFontSettings::apply();

    // Initialize css
    CSS::init();

    // Create backend manager
    InitializationManager::init(&app);

    // Show welcome window
    WindowManager::welcomeWindow()->show();

    // Start main event loop
    return app.exec();
}
