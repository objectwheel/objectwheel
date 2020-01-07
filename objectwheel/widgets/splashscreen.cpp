#include <splashscreen.h>
#include <QPainter>

SplashScreen::SplashScreen(const QPixmap& pixmap, Qt::WindowFlags windowFlags)
    : QSplashScreen(pixmap, windowFlags)
{
}

void SplashScreen::drawContents(QPainter* painter)
{
    QRectF r(0, height() - 20, width(), 20);
    r.adjust(0, -19, -18, -19);
    QFont f(font());
    f.setPixelSize(f.pixelSize() - 1);
    painter->setFont(f);
    painter->setPen(QColor(90, 90, 90));
    painter->drawText(r, message(), Qt::AlignRight | Qt::AlignVCenter);
}
