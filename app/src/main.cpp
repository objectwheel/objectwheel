#include <css.h>
#include <fit.h>
#include <components.h>
#include <mainwindow.h>
#include <QApplication>
#include <QFontDatabase>
#include <QtWebView>
#include <splashscreen.h>
#include <QIcon>
#include <QBuffer>

#define PIXEL_SIZE 12
#define REF_WIDTH 660
#define REF_HEIGHT 400
#define REF_DPI 102

int main(int argc, char *argv[])
{
	// Initialize application base with style
	QApplication a(argc, argv);
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
	QFontDatabase::addApplicationFont(":/resources/fonts/OpenSans-Bold.ttf");
	QFontDatabase::addApplicationFont(":/resources/fonts/OpenSans-Regular.ttf");
	QFontDatabase::addApplicationFont(":/resources/fonts/OpenSans-Light.ttf");
	QFontDatabase::addApplicationFont(":/resources/fonts/LiberationMono-Regular.ttf");
	QFont font("OpenSans");
	font.setPixelSize(Fit::fit(PIXEL_SIZE));
	QApplication::setFont(font);

	// Start MainWidget
	MainWindow w;
	QFile loading(":/resources/images/loading.gif");
	Q_ASSERT(loading.open(QIODevice::ReadOnly));
	auto data = loading.readAll();
	QBuffer buff(&data);
	buff.open(QBuffer::ReadOnly);

	// Init Splash Screen
	SplashScreen::init(&w);
	SplashScreen::setTextColor(Qt::white);
	SplashScreen::setBackgroundBrush(QColor("#2c9ecc"));
	SplashScreen::setText("Launching...");
//	SplashScreen::setIcon(QIcon(":/resources/images/logo.png"));
//	SplashScreen::setIconSize(Fit::fit(160), Fit::fit(80));
	SplashScreen::setLoadingSize(Fit::fit(160), Fit::fit(120));
	SplashScreen::setLoadingDevice(&buff);
	SplashScreen::show();

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
