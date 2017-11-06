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

#define PIXEL_SIZE 14
#define REF_WIDTH 1366
#define REF_HEIGHT 768
#define REF_DPI 144

//FIXME: Fit library

int main(int argc, char *argv[])
{
    // Init application
    QApplication a(argc, argv);

    // Setup OpenGL format
    QSurfaceFormat format;
    format.setSamples(4);
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

# if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
    //    Multiple instances protection
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

    // Init application settings
//    QApplication::setStyle("fusion");
    qputenv("QT_QUICK_CONTROLS_STYLE", "Base");
    qputenv("QML_DISABLE_DISK_CACHE", "true");
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setWindowIcon(QIcon(":/resources/images/owicon.png"));
    qsrand(QDateTime::currentMSecsSinceEpoch());

    // Init CSS
    CSS::init();

    // Init Components
    Components::init();

    // Initialize Web View
    QtWebView::initialize();

    // Start MainWidget
    MainWindow w;
# if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
    w.resize(qApp->primaryScreen()->availableSize());
    w.show();
# else
    Fit::init(REF_WIDTH, REF_HEIGHT, REF_DPI);
    w.showFullScreen();
# endif

    // Add system wide fonts and set default font
    QFont font;
    font.setPixelSize(Fit::fit(PIXEL_SIZE));
#if defined(Q_OS_MACOS)
    font.setFamily(".SF NS Display");
#endif
    QApplication::setFont(font);
    QFontDatabase::addApplicationFont(":/resources/fonts/LiberationMono-Regular.ttf");

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules(QStringLiteral("qtc*.info=false\n"
                                                    "qtc*.debug=false\n"
                                                    "qtc*.warning=false\n"
                                                    "qtc*.critical=false"));

    // Start main event loop
    return a.exec();
}
