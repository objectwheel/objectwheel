#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>

class SplashScreen final : public QSplashScreen
{
    Q_OBJECT
    Q_DISABLE_COPY(SplashScreen)

public:
    explicit SplashScreen(const QPixmap& pixmap, Qt::WindowFlags windowFlags);

private:
    void drawContents(QPainter* painter) override;
};

#endif // SPLASHSCREEN_H
