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

#define PIXEL_SIZE 13
#define REF_WIDTH 700
#define REF_HEIGHT 430
#define REF_DPI 127

/* Far tasks */
/*-----------*/
// Build gui equals (continuous task)
// Verisoning-Git integration
// Undo-redo
// Cloud part-sync-login etc
// Clear parser's ugly console outputs
// General bug fixing (continuous task)

/* Near tasks */
/*------------*/
// Fix qml editor according to dashboard database editing
// Fix parser errors
// Fix  parser slowness
// Import/Export project
// Add (secure)exit button
// Drag & Drop image files transforms into controls
// Fix Android's control drag & drop error
// Apply id changes into qml files

int main(int argc, char *argv[])
{
	// Init application
	QApplication a(argc, argv);

# if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
    // Multiple instances protection
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
	qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
	qsrand(QDateTime::currentMSecsSinceEpoch());

	// Init Fit
	Fit::init(REF_WIDTH, REF_HEIGHT, REF_DPI);

	// Init CSS
	CSS::init();

	// Init Components
	Components::init();

	// Initialize Web View
	QtWebView::initialize();

	// Add system wide fonts and set default font
	QFontDatabase::addApplicationFont(":/resources/fonts/LiberationMono-Regular.ttf");
	QFont font;
	font.setPixelSize(Fit::fit(PIXEL_SIZE));
	QApplication::setFont(font);

	// Start MainWidget
	MainWindow w;
# if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
	w.resize({REF_WIDTH, REF_HEIGHT});
	Fit::fit(&w, Fit::WidthHeight);
	w.show();
# else
	w.showFullScreen();
# endif

	// Start main event loop
	return a.exec();
}
