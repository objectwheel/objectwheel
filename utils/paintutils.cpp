#include <paintutils.h>

#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QWidget>

QPixmap PaintUtils::maskedPixmap(const QString& fileName, const QColor& color, const QWidget* widget)
{
    QImage source(fileName);
    source.setDevicePixelRatio(widget->devicePixelRatioF());

    QImage dest(source.size(), QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(widget->devicePixelRatioF());
    dest.fill(color);

    QPainter p(&dest);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(QRectF{{}, dest.size() / widget->devicePixelRatioF()}, source, source.rect());
    p.end();

    return QPixmap::fromImage(dest);
}