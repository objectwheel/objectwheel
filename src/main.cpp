#include <css.h>
#include <fit.h>
#include <components.h>
#include <mainwindow.h>
#include <QApplication>
#include <QFontDatabase>
#include <QtWebView>
#include <QIcon>
#include <QBuffer>
#include <QSharedMemory>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QTimer>
#include <QQuickStyle>

#define PIXEL_SIZE 13
#define REF_DPI 72.0

int main(int argc, char *argv[])
{
    // Init application
    QApplication a(argc, argv);
    QQuickStyle::setStyle("Material");

    # if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
    // Multiple instances protection
    QSharedMemory sharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory.create(1)) {
        sharedMemory.attach();
        sharedMemory.detach();
        if(!sharedMemory.create(1)) {
            QMessageBox::warning(NULL, "Quitting", "Another instance is already running.");
            a.exit();
            return 0;
        }
    }
    # endif

    // Initialize Fit library
    fit::update(REF_DPI);

    // Init application settings
    // QApplication::setStyle("fusion");
    qputenv("QT_QUICK_CONTROLS_STYLE", "Base");
    qputenv("QML_DISABLE_DISK_CACHE", "true");
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setWindowIcon(QIcon(":/resources/images/owicon.png"));

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
    QLoggingCategory::setFilterRules(QStringLiteral("qtc*.info=false\n"
      "qtc*.debug=false\n qtc*.warning=false\n qtc*.critical=false"));

    // Init CSS
    CSS::init();

    // Init Components
    Components::init();

    // Initialize Web View
    QtWebView::initialize();

    // Start MainWidget
    MainWindow w;
    # if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
    w.showMaximized();
    # else
    w.showFullScreen();
    # endif

    // Start main event loop
    return a.exec();
}
