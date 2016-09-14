#include <fitter.h>
#include <mainwindow.h>
#include <QApplication>
#include <QFontDatabase>

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

	// Initialize Scaling Fundamentals
	Fit::Setup({REF_WIDTH, REF_HEIGHT}, REF_DPI);

	// Add system wide fonts and set default font
	QFontDatabase::addApplicationFont(":/resources/fonts/raleway-light.ttf");
	QFont font("Raleway Light");
	font.setPixelSize(PIXEL_SIZE);
	QApplication::setFont(font);
	Fitter::AddAppWideWidgetFonts();

	// Start MainWidget
	MainWindow w;
# if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_WINPHONE)
	w.resize({REF_WIDTH, REF_HEIGHT});
	Fitter::AddWidget(&w, Fit::WidthHeight);
	w.show();
# else
	w.showFullScreen();
# endif

	// Start main event loop
	return a.exec();
}
