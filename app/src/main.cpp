#include <css.h>
#include <fit.h>
#include <components.h>
#include <mainwindow.h>
#include <scenemanager.h>
#include <QApplication>
#include <QFontDatabase>
#include <QtWebView>
#include <splashscreen.h>
#include <QIcon>
#include <QBuffer>
#include <QSharedMemory>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QTimer>

#define PIXEL_SIZE 13
#define REF_WIDTH 1000
#define REF_HEIGHT 600
#define REF_DPI 144

int main(int argc, char *argv[])
{
    // Init application
    QApplication a(argc, argv);

# if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
    //    Multiple instances protection
    QSharedMemory sharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
    if(!sharedMemory.create(1)) {
        sharedMemory.attach();
        sharedMemory.detach();
        if(!sharedMemory.create(1)) {
            QMessageBox::warning(NULL, "Quitting", "Another instance already running.");
            a.exit();
            return 0;
        }
    }
# endif

    // Init application settings
    QApplication::setStyle("fusion");
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
    w.resize({REF_WIDTH, REF_HEIGHT});
    Fit::fit(&w, Fit::WidthHeight);
    w.show();
# else
    Fit::init(REF_WIDTH, REF_HEIGHT, REF_DPI);
    w.showFullScreen();
# endif

    // Add system wide fonts and set default font
    QFont font;
    font.setPixelSize(Fit::fit(PIXEL_SIZE));
    QApplication::setFont(font);
    QFontDatabase::addApplicationFont(":/resources/fonts/LiberationMono-Regular.ttf");

    /* Disable Qml Parser warnings */
    QLoggingCategory::setFilterRules(QStringLiteral("qtc*.info=false\n"
                                                    "qtc*.debug=false\n"
                                                    "qtc*.warning=false\n"
                                                    "qtc*.critical=false"));

    QTimer::singleShot(1000, [&] { w.raise(); });

    // Start main event loop
    return a.exec();
}
