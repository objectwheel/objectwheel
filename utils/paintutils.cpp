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

QImage PaintUtils::renderFilledImage(const QSizeF& size, const QColor& fillColor, qreal dpr)
{
    QImage dest((size * dpr).toSize(), QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(dpr);
    dest.fill(fillColor);
    return dest;
}

QImage PaintUtils::renderTransparentImage(const QSizeF& size, qreal dpr)
{
    return renderFilledImage(size, Qt::transparent, dpr);
}

QImage PaintUtils::renderInvisibleControlImage(const QSizeF& size, qreal dpr)
{
    QImage dest = renderTransparentImage(size, dpr);

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

QImage PaintUtils::renderInitialControlImage(const QSizeF& size, qreal dpr)
{
    QImage dest = renderTransparentImage(size, dpr);

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

QImage PaintUtils::renderErrorControlImage(const QSizeF& size, qreal dpr)
{
    QImage dest = renderTransparentImage(size, dpr);

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

QImage PaintUtils::renderNonGuiControlImage(const QPixmap& pixmap, const QSizeF& size, qreal dpr)
{
    QImage dest = renderTransparentImage(size, dpr);
    QRectF destRect{{}, size};

    QImage source(pixmap.toImage());
    source.setDevicePixelRatio(dpr);
    QRectF sourceRect{{}, QSizeF{24, 24}};
    sourceRect.moveCenter(destRect.center());

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(sourceRect, source, source.rect()); // Note: The image is scaled to fit the rectangle
    p.end();

    return dest;
}

QIcon PaintUtils::renderOverlaidIcon(const QString& fileName, const QColor& color, qreal dpr)
{
    QIcon icon;
    icon.addPixmap(renderOverlaidPixmap(fileName, color, dpr));
    return icon;
}

QIcon PaintUtils::renderOverlaidIcon(const QIcon& icon, const QSize& size, const QColor& color, const QWidget* widget)
{
    QIcon i;
    Q_ASSERT(UtilityFunctions::window(widget));
    i.addPixmap(renderOverlaidPixmap(icon.pixmap(UtilityFunctions::window(widget), size), color, widget->devicePixelRatioF()));
    return i;
}

QIcon PaintUtils::renderButtonIcon(const QString& fileName, qreal dpr)
{
    QIcon icon;
    QPixmap pixmap(fileName);
    pixmap.setDevicePixelRatio(dpr);
    icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderOverlaidPixmap(fileName, "#20000000", dpr), QIcon::Normal, QIcon::On);
    return icon;
}

QIcon PaintUtils::renderToolButtonIcon(const QString& fileName, qreal dpr)
{
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
    QColor up = widget->palette().color(QPalette::Active, QPalette::ButtonText);
    QColor down = widget->palette().color(QPalette::Active, QPalette::ButtonText).darker();
    icon.addPixmap(renderOverlaidPixmap(fileName, up, widget->devicePixelRatioF()), QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderOverlaidPixmap(fileName, down, widget->devicePixelRatioF()), QIcon::Normal, QIcon::On);
    return icon;
}

QPixmap PaintUtils::renderOverlaidPixmap(const QString& fileName, const QColor& color, qreal dpr)
{
    QPixmap source(fileName);
    source.setDevicePixelRatio(dpr);
    return renderOverlaidPixmap(source, color, dpr);
}

QPixmap PaintUtils::renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color, qreal dpr)
{
    QPixmap dest(pixmap);
    dest.setDevicePixelRatio(dpr);

    QColor opaque(color);
    opaque.setAlphaF(1);

    QImage overlay = renderFilledImage(pixmap.size() / dpr, opaque, dpr);
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

// For colorizing the files that are located within the "utils/images" directory
QPixmap PaintUtils::renderMaskedPixmap(const QString& fileName, const QColor& color, qreal dpr)
{
    QImage source(fileName);
    source.setDevicePixelRatio(dpr);

    QImage dest = renderFilledImage(source.size() / dpr, Qt::transparent, dpr);

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

QPixmap PaintUtils::renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, qreal dpr)
{
    QImage dest = renderFilledImage(size, color, dpr);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::NoBrush);
    p.setPen(pen);
    p.drawRect(QRectF{{}, size}.adjusted(0.5, 0.5, -0.5, -0.5));
    p.end();

    return QPixmap::fromImage(dest);
}

void PaintUtils::drawPanelButtonBevel(QPainter* painter, const QStyleOptionButton& option, bool downWhenChecked)
{
    painter->save();

    const bool down = (option.state & QStyle::State_Sunken)
            || (downWhenChecked && (option.state & QStyle::State_On));
    const bool bright = option.styleObject
            ? option.styleObject->property("ow_bottombar_bright").toBool() : false;

    // Draw drop shadow
    QLinearGradient shadowGrad(0, 0, 1, 0);
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
    QLinearGradient darkGrad(0, 0, 0, 1);
    darkGrad.setCoordinateMode(QGradient::ObjectMode);
    darkGrad.setColorAt(0.85, "#20303030");
    darkGrad.setColorAt(1, "#3f000000");
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(QRectF(option.rect).adjusted(0, 0.5, 0, -0.5), 4, 4);
    painter->setPen(Qt::NoPen);
    painter->setBrush(darkGrad);
    painter->drawPath(shadowPath);

    // Draw body
    QLinearGradient midGrad(0, 0, 0, 1);
    midGrad.setCoordinateMode(QGradient::ObjectMode);
    if (bright) {
        midGrad.setColorAt(0, "#b34b46");
        midGrad.setColorAt(1, "#a2403b");
    } else {
        midGrad.setColorAt(0, "#e4e4e4");
        midGrad.setColorAt(1, "#dedede");
    }
    QLinearGradient buttonGrad(0, 0, 0, 1);
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

    if ((option.features & QStyleOptionButton::HasMenu)
            && ((option.state & QStyle::State_MouseOver) || (option.state & QStyle::State_Sunken))) {
        int mbi = qApp->style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option);
        QRectF ir = option.rect;
        QStyleOptionButton newBtn = option;
        newBtn.rect = QRect(ir.right() - mbi, ir.bottom() - mbi, mbi, mbi);
        qApp->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, painter);
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

bool PaintUtils::isBlankImage(const QImage& image)
{
    if (image.isNull())
        return true;

    int totalAlpha = 150 * 8;
    const int hspacing = 8;
    const int wspacing = qRound(qMax(hspacing * qreal(image.width()) / image.height(), 1.0));

    for (int w = 0, h = image.height() / 2.0; w < image.width(); w += wspacing)
        totalAlpha -= qAlpha(image.pixel(w, h));

    if (totalAlpha < 0)
        return false;

    for (int w = 0, h = 0; w < image.width() && h < image.height(); w += wspacing, h += hspacing)
        totalAlpha -= qAlpha(image.pixel(w, h));

    if (totalAlpha < 0)
        return false;

    for (int w = image.width() - 1, h = 0; w >= 0 && h < image.height(); w -= wspacing, h += hspacing)
        totalAlpha -= qAlpha(image.pixel(w, h));

    return totalAlpha > 0;
}
