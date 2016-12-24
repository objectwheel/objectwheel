#include <css.h>
#include <fit.h>
#include <components.h>
#include <mainwindow.h>
#include <QApplication>
#include <QFontDatabase>
#include <QtWebView>

#define PIXEL_SIZE 14
#define REF_WIDTH 660
#define REF_HEIGHT 400
#define REF_DPI 102

int main(int argc, char *argv[])
{
	// Initialize application base with style
	QApplication a(argc, argv);
	QApplication::setStyle("fusion");
	qputenv("QT_QUICK_CONTROLS_STYLE", "Base");

	// Init Fit
	Fit::init(REF_WIDTH, REF_HEIGHT, REF_DPI);

	// Init CSS
	CSS::init();

	// Init Components
	Components::init();

	// Initialize Web View
	QtWebView::initialize();

	// Add system wide fonts and set default font
	QFontDatabase::addApplicationFont(":/resources/fonts/OpenSans-Regular.ttf");
	QFont font("OpenSans");
	font.setPixelSize(Fit::fit(PIXEL_SIZE));
	QApplication::setFont(font);

	// Start MainWidget
	MainWindow w;
# if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
	w.resize({REF_WIDTH, REF_HEIGHT});
	fit(&w, Fit::WidthHeight);
	w.show();
# else
	w.showFullScreen();
# endif

	// Start main event loop
	return a.exec();
}
