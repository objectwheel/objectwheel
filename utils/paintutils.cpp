#include <paintutils.h>

#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QApplication>

QPixmap PaintUtils::maskedPixmap(const QString& fileName, const QColor& color, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage source(fileName);
    source.setDevicePixelRatio(dpr);

    QImage dest(source.size(), QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(dpr);
    dest.fill(color);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(QRectF{{}, dest.size() / dpr}, source, source.rect());
    p.end();

    return QPixmap::fromImage(dest);
}

QPixmap PaintUtils::colorToPixmap(const QSize& size, const QColor& color, const QWidget* widget,
                                 bool hasBorder)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage dest(size * dpr, QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(dpr);
    dest.fill(color);

    if (hasBorder) {
        QPainter p(&dest);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::black);
        p.drawRect(QRectF{{}, size}.adjusted(0.5, 0.5, -0.5, -0.5));
        p.end();
    }

    return QPixmap::fromImage(dest);
}