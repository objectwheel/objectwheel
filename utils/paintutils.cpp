#include <paintutils.h>

#include <QPainter>
#include <QImage>
#include <QBitmap>
#include <QWidget>
#include <QApplication>
#include <QStyleOptionButton>

QImage PaintUtils::renderFilledImage(const QSizeF& size, const QColor& fillColor, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage dest((size * dpr).toSize(), QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(dpr);
    dest.fill(fillColor);
    return dest;
}

QImage PaintUtils::renderTransparentImage(const QSizeF& size, const QWidget* widget)
{
    return renderFilledImage(size, Qt::transparent, widget);
}

QImage PaintUtils::renderInvisibleControlImage(const QSizeF& size, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage dest = renderTransparentImage(size, widget);

    QBitmap bitmap(":/images/texture.bmp");
    bitmap.setDevicePixelRatio(dpr);

    QBrush brush("#d0d0d0", bitmap);
    brush.setStyle(Qt::TexturePattern);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(brush);
    p.setPen("#bdbdbd");
    p.drawRect(QRectF{{}, size}.adjusted(0.5, 0.5, -0.5, -0.5));
    p.end();

    return dest;
}

QImage PaintUtils::renderInitialControlImage(const QSizeF& size, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage dest = renderTransparentImage(size, widget);

    QImage source(":/images/wait.png");
    source.setDevicePixelRatio(dpr);

    QRectF destRect{{}, size};
    QRectF sourceRect{{}, QSizeF{24, 24}};
    sourceRect.moveCenter(destRect.center());

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(sourceRect, source, source.rect());
    p.end();

    return dest;
}

QImage PaintUtils::renderErrorControlImage(const QSizeF& size, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage dest = renderTransparentImage(size, widget);

    QImage source(":/images/error.png");
    source.setDevicePixelRatio(dpr);

    QRectF destRect{{}, size};
    QRectF sourceRect{{}, QSizeF{24, 24}};
    sourceRect.moveCenter(destRect.center());

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(sourceRect, source, source.rect());
    p.end();

    return dest;
}

QImage PaintUtils::renderNonGuiControlImage(const QString& url, const QSizeF& size, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage dest = renderTransparentImage(size, widget);

    QImage source(url);
    source.setDevicePixelRatio(dpr);

    QRectF destRect{{}, size};
    QRectF sourceRect{{}, QSizeF{24, 24}};
    sourceRect.moveCenter(destRect.center());

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(sourceRect, source, source.rect());
    p.end();

    return dest;
}

QPixmap PaintUtils::renderColorizedPixmap(const QString& fileName, const QColor& color, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage source(fileName);
    source.setDevicePixelRatio(dpr);

    QImage dest = renderFilledImage(source.size() / dpr, color, widget);
    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(QRectF{{}, dest.size() / dpr}, source, source.rect());
    p.end();

    return QPixmap::fromImage(dest);
}

// For colorizing the files that are located within the "utils/images" directory
QPixmap PaintUtils::renderMaskedPixmap(const QString& fileName, const QColor& color, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QImage source(fileName);
    source.setDevicePixelRatio(dpr);

    QImage dest = renderFilledImage(source.size() / dpr, Qt::transparent, widget);

    Q_ASSERT(source.size() == dest.size());

    QColor copy(color);
    for (int i = 0; i < dest.width(); ++i) {
        for (int j = 0; j < dest.height(); ++j) {
            int alpha = QColor(source.pixel(i, j)).black();
            copy.setAlpha(alpha);
            dest.setPixelColor(QPoint(i, j), copy);
        }
    }

    return QPixmap::fromImage(dest);
}

QPixmap PaintUtils::renderColorPixmap(const QSize& size, const QColor& color, const QPen& pen, const QWidget* widget)
{
    QImage dest = renderFilledImage(size, color, widget);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::NoBrush);
    p.setPen(pen);
    p.drawRect(QRectF{{}, size}.adjusted(0.5, 0.5, -0.5, -0.5));
    p.end();

    return QPixmap::fromImage(dest);
}

void PaintUtils::drawMacStyleButtonBackground(QPainter* painter, const QStyleOption& option, QWidget* widget)
{
    painter->save();

    // Draw drop shadow
    QPainterPath dropShadowPath;
    dropShadowPath.addRect(3.5, widget->rect().bottom() + 0.5, widget->width() - 7, 1);
    QLinearGradient dropShadowGrad(QPointF(3.5, widget->rect().bottom()),
                                   QPointF(widget->width() - 3.5, widget->rect().bottom()));
    dropShadowGrad.setColorAt(0, QColor("#12202020"));
    dropShadowGrad.setColorAt(0.05, QColor("#10202020"));
    dropShadowGrad.setColorAt(0.5, QColor("#10202020"));
    dropShadowGrad.setColorAt(0.95, QColor("#10202020"));
    dropShadowGrad.setColorAt(1, QColor("#12202020"));
    painter->setPen(Qt::NoPen);
    painter->setBrush(dropShadowGrad);
    painter->drawPath(dropShadowPath);

    // Draw shadow
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(QRectF(widget->rect()).adjusted(0, 0.5, 0, -0.5), 4.25, 4.25);
    QLinearGradient shadowGrad(widget->rect().topLeft(), widget->rect().bottomLeft());
    shadowGrad.setColorAt(0.85, QColor("#20303030"));
    shadowGrad.setColorAt(1, QColor("#3d000000"));
    painter->setPen(Qt::NoPen);
    painter->setBrush(shadowGrad);
    painter->drawPath(shadowPath);

    const bool down = (option.state & QStyle::State_Sunken) || (option.state & QStyle::State_On);

    // Draw body
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(widget->rect()).adjusted(0.5, 1, -0.5, -1), 3.65, 3.65);
    QLinearGradient bodyGrad(widget->rect().topLeft(), widget->rect().bottomLeft());
    bodyGrad.setColorAt(0, down ? QColor("#e7e7e7") : QColor("#fefefe"));
    bodyGrad.setColorAt(1, down ? QColor("#e1e1e1") : QColor("#f7f7f7"));
    painter->setPen(Qt::NoPen);
    painter->setBrush(bodyGrad);
    painter->drawPath(bodyPath);
    // NOTE: QRect's bottom() and bottomLeft()... are different from QRectF, for historical reasons
    // as stated in the docs, those functions return top() + height() - 1 (QRect)

    // Draw glowing for pressed state
    if (down) {
        QPainterPath glowPath;
        glowPath.addRoundedRect(QRectF(widget->rect()).adjusted(0.5, 2, -0.5, 1), 3.65, 3.65);
        QLinearGradient glowGrad(widget->width() / 2.0, 1, widget->width() / 2.0, 2);
        glowGrad.setColorAt(0, QColor("#f7f7f7"));
        glowGrad.setColorAt(1, QColor("#eaeaea"));
        painter->setPen(Qt::NoPen);
        painter->setBrush(glowGrad);
        painter->drawPath(bodyPath.subtracted(glowPath));
    }

    painter->restore();
}

void PaintUtils::drawMenuDownArrow(QPainter* painter, const QPointF& offset, const QStyleOption& option, QWidget* widget)
{
    painter->save();
    const QColor& color = (option.state & QStyle::State_Sunken)
            ? widget->palette().text().color().lighter(115)
            : widget->palette().text().color().lighter(160);
    QPointF points[3] = {{0, 0}, {4.5, 0}, {2.25, 2.5}};
    points[0] += offset; points[1] += offset; points[2] += offset;
    painter->setPen(color);
    painter->setBrush(color);
    painter->drawPolygon(points, 3);
    painter->restore();
}

QIcon PaintUtils::renderColorizedIcon(const QString& fileName, const QColor& color, const QWidget* widget)
{
    QIcon icon;
    icon.addPixmap(renderColorizedPixmap(fileName, color, widget));
    return icon;
}
