#include <css.h>
#include <fit.h>
#include <components.h>
#include <windowmanager.h>
#include <menumanager.h>
#include <backendmanager.h>
#include <filemanager.h>

#include <QApplication>
#include <QFontDatabase>
#include <QtWebView>
#include <QIcon>
#include <QSharedMemory>
#include <QMessageBox>
#include <QLoggingCategory>

#define PIXEL_SIZE 14
#define MIN_DPI 110.0
#define REF_DPI 149.0

int main(int argc, char *argv[])
{
    // Boot settings
    qputenv("QT_QUICK_CONTROLS_STYLE", "Base");
    qputenv("QML_DISABLE_DISK_CACHE", "true");
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

    // Initialize application
    QApplication a(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setOrganizationName(APP_CORP);
    QApplication::setApplicationVersion(APP_VER);
    QApplication::setOrganizationDomain(APP_DOMAIN);
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setWindowIcon(QIcon(":/resources/images/owicon.png"));

    // Multiple instances protection
    # if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
    QSharedMemory sharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory.create(1)) {
        sharedMemory.attach();
        sharedMemory.detach();
        if(!sharedMemory.create(1)) {
            QMessageBox::warning(nullptr, "Quitting", "Another instance is already running.");
            a.exit();
            return 0;
        }
    }
    # endif

    // Initialize fit library
    fit::update(REF_DPI, MIN_DPI);

    // Font settings
    for (const auto& font : lsfile(":/resources/fonts"))
        QFontDatabase::addApplicationFont(":/resources/fonts/" + font);

    QFont font;
    font.setPixelSize(fit::fx(PIXEL_SIZE));
    #if defined(Q_OS_MACOS)
    font.setFamily(".SF NS Display");
    #elif defined(Q_OS_WIN)
    font.setFamily("Segoe UI");
    #else
    font.setFamily("Open Sans");
    #endif
    QApplication::setFont(font);

    // Disable Qml parser warnings
    QLoggingCategory::setFilterRules(
        QStringLiteral(
            "qtc*.info=false\n"
            "qtc*.debug=false\n"
            "qtc*.warning=false\n"
            "qtc*.critical=false"
        )
    );

    // Initialize css
    CSS::init();

    // Initialize components
    Components::init();

    // Initialize web view
    QtWebView::initialize();

    // Create backend manager
    BackendManager::instance()->init();

    // Show welcome window
    WindowManager::instance()->show(WindowManager::Welcome);

    // Initialize menus
    MenuManager::instance()->init();

    // Start main event loop
    return a.exec();
}
