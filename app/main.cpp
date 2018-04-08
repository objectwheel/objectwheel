#include <css.h>
#include <fit.h>
#include <windowmanager.h>
#include <menumanager.h>
#include <backendmanager.h>
#include <filemanager.h>

#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include <QSharedMemory>
#include <QMessageBox>

#define PIXEL_SIZE 14
#define MIN_DPI 110.0
#define REF_DPI 149.0

int main(int argc, char* argv[])
{
    // Boot Settings
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL); // For reCaptcha

    // Initialize application
    QApplication a(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setOrganizationName(APP_CORP);
    QApplication::setApplicationVersion(APP_VER);
    QApplication::setOrganizationDomain(APP_DOMAIN);
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setWindowIcon(QIcon(":/resources/images/owicon.png"));

    // Multiple instances protection
    QSharedMemory sharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory.create(1)) {
        sharedMemory.attach();
        sharedMemory.detach();
        if(!sharedMemory.create(1)) {
            QMessageBox::warning(
                nullptr,
                QObject::tr("Quitting"),
                QObject::tr("Another instance is already running.")
            );
            a.exit();
            return 0;
        }
    }

    // Initialize fit library
    fit::update(REF_DPI, MIN_DPI);

    // Font settings
    for (const auto& font : lsfile(":/resources/fonts"))
        QFontDatabase::addApplicationFont(":/resources/fonts/" + font);

    QFont font;
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setPixelSize(fit::fx(PIXEL_SIZE));
    #if defined(Q_OS_MACOS)
    font.setFamily(".SF NS Display");
    #elif defined(Q_OS_WIN)
    font.setFamily("Segoe UI");
    #else
    font.setFamily("Open Sans");
    #endif
    QApplication::setFont(font);

    // Initialize css
    CSS::init();

    // Create backend manager
    BackendManager::instance()->init();

    // Show welcome window
    WindowManager::instance()->show(WindowManager::Welcome);

    // Initialize menus
    MenuManager::instance()->init();

    // Start main event loop
    return a.exec();
}
