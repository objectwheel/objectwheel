#include <splashscreen.h>
#include <windowmanager.h>
#include <welcomewindow.h>
#include <QApplication>

SplashScreen::SplashScreen(QWidget *parent) : QSplashScreen(parent)
{
    QPixmap pixmap(":/images/splash.png");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    setPixmap(pixmap.scaled(int(512 * devicePixelRatioF()),
                            int(280 * devicePixelRatioF()),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    show();
    QApplication::processEvents();
}

SplashScreen::~SplashScreen()
{
    finish(WindowManager::welcomeWindow());
}
