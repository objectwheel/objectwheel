#include <paintutils.h>
#include <utilityfunctions.h>

#include <QByteArray>
#include <QPainter>
#include <QImage>
#include <QBitmap>
#include <QWidget>
#include <QApplication>
#include <QStyleOption>
#include <QPalette>
#include <QFileInfo>

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

QImage PaintUtils::renderNonGuiControlImage(const QByteArray& data, const QSizeF& size, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();

    QImage dest = renderTransparentImage(size, widget);
    QRectF destRect{{}, size};

    QImage source(QImage::fromData(data));
    source.setDevicePixelRatio(dpr);
    QRectF sourceRect{{}, QSizeF{24, 24}};
    sourceRect.moveCenter(destRect.center());

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(sourceRect, source, source.rect()); // Note: The image is scaled to fit the rectangle
    p.end();

    return dest;
}

QIcon PaintUtils::renderOverlaidIcon(const QString& fileName, const QColor& color, const QWidget* widget)
{
    QIcon icon;
    icon.addPixmap(renderOverlaidPixmap(fileName, color, widget));
    return icon;
}

QIcon PaintUtils::renderOverlaidIcon(const QIcon& icon, const QSize& size, const QColor& color, const QWidget* widget)
{
    QIcon i;
    Q_ASSERT(UtilityFunctions::window(widget));
    i.addPixmap(renderOverlaidPixmap(icon.pixmap(UtilityFunctions::window(widget), size), color, widget));
    return i;
}

QIcon PaintUtils::renderButtonIcon(const QString& fileName, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QIcon icon;
    QPixmap pixmap(fileName);
    pixmap.setDevicePixelRatio(dpr);
    icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderOverlaidPixmap(fileName, "#20000000", widget), QIcon::Normal, QIcon::On);
    return icon;
}

QIcon PaintUtils::renderToolButtonIcon(const QString& fileName, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QFileInfo fileInfo(fileName);
    QPixmap off, on;

    off.load(fileName);
    on.load(fileInfo.path() + '/' + fileInfo.baseName() + "-active." + fileInfo.suffix());
    off.setDevicePixelRatio(dpr);
    on.setDevicePixelRatio(dpr);

    QIcon icon;
    icon.addPixmap(off, QIcon::Normal, QIcon::Off);
    icon.addPixmap(on, QIcon::Normal, QIcon::On);
    return icon;
}

QIcon PaintUtils::renderOverlaidButtonIcon(const QString& fileName, const QWidget* widget)
{
    QIcon icon;
    QColor up = widget->palette().buttonText().color();
    QColor down = widget->palette().buttonText().color().darker();
    icon.addPixmap(renderOverlaidPixmap(fileName, up, widget), QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderOverlaidPixmap(fileName, down, widget), QIcon::Normal, QIcon::On);
    return icon;
}

QPixmap PaintUtils::renderOverlaidPixmap(const QString& fileName, const QColor& color, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QPixmap source(fileName);
    source.setDevicePixelRatio(dpr);
    return renderOverlaidPixmap(source, color, widget);
}

QPixmap PaintUtils::renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();

    QPixmap dest(pixmap);
    dest.setDevicePixelRatio(dpr);

    QColor opaque(color);
    opaque.setAlphaF(1);

    QImage overlay = renderFilledImage(pixmap.size() / dpr, opaque, widget);
    {
        QPainter p(&overlay);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRectF({}, pixmap.size() / dpr), pixmap, pixmap.rect());
    }

    if (color.alphaF() == 1)
        return QPixmap::fromImage(overlay);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setOpacity(color.alphaF());
    p.drawImage(QRectF({}, dest.size() / dpr), overlay, overlay.rect());
    p.end();

    return dest;
}

QPixmap PaintUtils::renderOverlaidPixmapFromData(const QByteArray& data, const QColor& color, const QWidget* widget)
{
    qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QPixmap source = QPixmap::fromImage(QImage::fromData(data));
    source.setDevicePixelRatio(dpr);
    return renderOverlaidPixmap(source, color, widget);
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

QPixmap PaintUtils::renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, const QWidget* widget)
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

void PaintUtils::drawPanelButtonBevel(QPainter* painter, const QStyleOption& option)
{
    painter->save();

    const bool down = (option.state & QStyle::State_Sunken) || (option.state & QStyle::State_On);
    const bool bright = option.styleObject->property("ow_bottombar_bright").toBool();

    // Draw drop shadow
    QLinearGradient shadowGrad({0.0, 0.5}, {1.0, 0.5});
    shadowGrad.setCoordinateMode(QGradient::ObjectMode);
    shadowGrad.setColorAt(0, "#12202020");
    shadowGrad.setColorAt(0.05, "#10202020");
    shadowGrad.setColorAt(0.5, "#10202020");
    shadowGrad.setColorAt(0.95, "#10202020");
    shadowGrad.setColorAt(1, "#12202020");
    QPainterPath dropShadowPath;
    dropShadowPath.addRect(3.5, option.rect.bottom() + 0.5, option.rect.width() - 7, 1);
    painter->setPen(Qt::NoPen);
    painter->setBrush(shadowGrad);
    painter->drawPath(dropShadowPath);

    // Draw shadow
    QLinearGradient darkGrad({0.0, 0.0}, {0.0, 1.0});
    darkGrad.setCoordinateMode(QGradient::ObjectMode);
    darkGrad.setColorAt(0.85, "#20303030");
    darkGrad.setColorAt(1, "#3f000000");
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(QRectF(option.rect).adjusted(0, 0.5, 0, -0.5), 4, 4);
    painter->setPen(Qt::NoPen);
    painter->setBrush(darkGrad);
    painter->drawPath(shadowPath);

    // Draw body
    QLinearGradient midGrad({0.0, 0.0}, {0.0, 1.0});
    midGrad.setCoordinateMode(QGradient::ObjectMode);
    if (bright) {
        midGrad.setColorAt(0, "#b34b46");
        midGrad.setColorAt(1, "#a2403b");
    } else {
        midGrad.setColorAt(0, "#e4e4e4");
        midGrad.setColorAt(1, "#dedede");
    }
    QLinearGradient buttonGrad({0.0, 0.0}, {0.0, 1.0});
    buttonGrad.setCoordinateMode(QGradient::ObjectMode);
    if (bright) {
        buttonGrad.setColorAt(0, "#c2504b");
        buttonGrad.setColorAt(1, "#b34b46");
    } else {
        buttonGrad.setColorAt(0, "#fdfdfd");
        buttonGrad.setColorAt(1, "#f3f3f3");
    }
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(option.rect).adjusted(0.5, 1, -0.5, -1), 3.5, 3.5);
    painter->setPen(Qt::NoPen);
    painter->setBrush(down ? midGrad : buttonGrad);
    painter->drawPath(bodyPath);
    // NOTE: QRect's bottom() and bottomLeft()... are different from QRectF, for historical reasons
    // as stated in the docs, those functions return top() + height() - 1 (QRect)

    // Draw glowing for pressed state
    if (down) {
        QPainterPath glowPathUp;
        glowPathUp.addRoundedRect(QRectF(option.rect).adjusted(0, 1.5, 0, 1), 4.5, 4.5);
        QPainterPath glowPathDown;
        glowPathDown.addRoundedRect(QRectF(option.rect).adjusted(0, 2, 0, 1), 4.5, 4.5);
        painter->setPen(Qt::NoPen);
        if (bright) {
            painter->setBrush(QColor("#cc5650"));
            painter->drawPath(bodyPath.subtracted(glowPathDown));
            painter->setBrush(QColor("#e5615a"));
        } else {
            painter->setBrush(QColor("#ebebeb"));
            painter->drawPath(bodyPath.subtracted(glowPathDown));
            painter->setBrush(QColor("#f3f3f3"));
        }
        painter->drawPath(bodyPath.subtracted(glowPathUp));
    }

    painter->restore();
}

void PaintUtils::drawSearchEditBevel(QPainter* painter, const QStyleOption& option)
{
    painter->save();

    // Draw outline
    QPainterPath outlinePath;
    outlinePath.addRoundedRect(option.rect, 4, 4);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#b1b1b1"));
    painter->drawPath(outlinePath);

    // Draw inline
    QPainterPath inlinePath;
    inlinePath.addRoundedRect(QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5), 3.5, 3.5);
    painter->setBrush(QColor("#f0f0f0"));
    painter->drawPath(inlinePath);

    // Draw body
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(option.rect).adjusted(1, 1, -1, -1), 3, 3);
    painter->setBrush(QColor("#ffffff"));
    painter->drawPath(bodyPath);

    painter->restore();
}
