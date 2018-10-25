#include <splashscreen.h>
#include <windowmanager.h>
#include <welcomewindow.h>
#include <QApplication>

SplashScreen::SplashScreen(QWidget *parent) : QSplashScreen(parent)
{
    QPixmap pixmap(":/images/splash.png");
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    setPixmap(pixmap);
    show();
    QApplication::processEvents();
}

SplashScreen::~SplashScreen()
{
    finish(WindowManager::welcomeWindow());
}