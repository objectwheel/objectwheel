#include <css.h>
#include <fit.h>
#include <components.h>
#include <windowmanager.h>
#include <menumanager.h>
#include <backendmanager.h>

#include <QApplication>
#include <QFontDatabase>
#include <QtWebView>
#include <QIcon>
#include <QSharedMemory>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QQuickStyle>
#include <QSurfaceFormat>

#define PIXEL_SIZE 13
#define REF_DPI 72.0

int main(int argc, char *argv[])
{
    // Init application
    QApplication a(argc, argv);

    // Init application settings
    QApplication::setOrganizationName(APP_CORP);
    QApplication::setOrganizationDomain(APP_DOMAIN);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setApplicationVersion(APP_VER);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setWindowIcon(QIcon(":/resources/images/owicon.png"));

    qputenv("QT_QUICK_CONTROLS_STYLE", "Base");
    qputenv("QML_DISABLE_DISK_CACHE", "true");

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

    // Initialize Fit library
    fit::update(REF_DPI);

    // Setup OpenGL format
    QSurfaceFormat format;
    format.setSamples(4);
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    // Add system wide fonts and set default font
    QFont font;
    font.setPixelSize(fit::fx(PIXEL_SIZE));
    #if defined(Q_OS_MACOS)
    font.setFamily(".SF NS Display");
    #elif defined(Q_OS_WIN)
    font.setFamily("Segoe UI");
    #endif
    QApplication::setFont(font);
    QFontDatabase::addApplicationFont(":/resources/fonts/LiberationMono-Regular.ttf");

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules(
        QStringLiteral(
            "qtc*.info=false\n"
            "qtc*.debug=false\n"
            "qtc*.warning=false\n"
            "qtc*.critical=false"
        )
    );

    // Init CSS
    CSS::init();

    // Init Components
    Components::init();

    // Initialize Web View
    QtWebView::initialize();

    // Initialize Menus
    MenuManager::instance();

    // Start Windows
    WindowManager::instance()->show(WindowManager::Welcome);

    BackendManager::instance();

    // Start main event loop
    return a.exec();
}
