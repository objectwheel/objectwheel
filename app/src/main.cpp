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
#include <QSharedMemory>
#include <QMessageBox>

#define PIXEL_SIZE 13
#define REF_WIDTH 700
#define REF_HEIGHT 430
#define REF_DPI 127

// m_Items.removeAt ve deleteLater olan her yerde context'i temizlemeyi unutma (ilgili nesneyi 0 layarak)
// m_Items.removeAt ve deleteLater olan her yerde removeParentalRelations yapmayı unutma
// removeParentalRelations olan her yerde removeSave yapmayı unutma

int main(int argc, char *argv[])
{
	// Init application
	QApplication a(argc, argv);

	// Multiple instances protection
	QSharedMemory sharedMemory("T2JqZWN0d2hlZWxTaGFyZWRNZW1vcnlLZXk");
	if(!sharedMemory.create(1)) {
		sharedMemory.attach();
		sharedMemory.detach();
		if(!sharedMemory.create(1)) {
		   QMessageBox::warning(NULL, "Warning!", "Another instance already running!");
		   a.exit(); // exit already a process running
		   return 0;
		}
	}

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
	QFile loading(":/resources/images/loading.gif");
	if (!loading.open(QIODevice::ReadOnly)) qFatal("main : Error occurred");
	auto data = loading.readAll();
	QBuffer buff(&data);
	buff.open(QBuffer::ReadOnly);

	// Init Splash Screen
	SplashScreen::init(&w);
	SplashScreen::setTextColor(Qt::white);
	SplashScreen::setBackgroundBrush(QColor("#262626"));
	SplashScreen::setIcon(QIcon(":/resources/images/logo.png"));
	SplashScreen::setIconSize(Fit::fit(160), Fit::fit(80));
	SplashScreen::setLoadingSize(Fit::fit(30), Fit::fit(30));
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
