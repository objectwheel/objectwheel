#include <fit.h>
#include <mainwindow.h>
#include <QApplication>
#include <QFontDatabase>

#define REF_WIDTH 620
#define REF_HEIGHT 400
#define REF_DPI 96

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
	font.setPixelSize(fit(15));
	QApplication::setFont(font);

	// Start MainWidget
	MainWindow w;
	w.resize(REF_WIDTH, REF_HEIGHT);
	w.show();

	// Start main event loop
	return a.exec();
}
