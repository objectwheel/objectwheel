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
#include <QWindow>

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

QImage PaintUtils::renderNonGuiControlImage(const QString& imagePath, const QSizeF& size, void* widget)
{
    QWindow* window = UtilityFunctions::window((QWidget*)widget);
    const qreal dpr = window ? window->devicePixelRatio() : qApp->devicePixelRatio();
    const QSize iconSize(16, 16);

    QImage dest = renderTransparentImage(size, dpr);
    QImage source(QIcon(imagePath).pixmap(window, iconSize).toImage());
    source.setDevicePixelRatio(dpr);

    QRectF destRect({}, size);
    QRectF sourceRect({}, iconSize);
    sourceRect.moveCenter(destRect.center());

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawImage(sourceRect, source, source.rect()); // Note: The image is scaled to fit the rectangle
    p.end();

    return dest;
}

QIcon PaintUtils::renderButtonIcon(const QString& fileName, const QPalette& palette)
{
    QIcon icon;
    QColor up = palette.color(QPalette::Normal, QPalette::ButtonText);
    QColor down("#157efb");
    icon.addPixmap(renderOverlaidPixmap(fileName, up, 1), QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderOverlaidPixmap(fileName, down, 1), QIcon::Normal, QIcon::On);
    return icon;
}

QPixmap PaintUtils::renderOverlaidPixmap(const QString& fileName, const QColor& color, qreal dpr)
{
    QPixmap source(fileName);
    source.setDevicePixelRatio(dpr);
    return renderOverlaidPixmap(source, color);
}

QPixmap PaintUtils::renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color)
{
    QPixmap dest(pixmap);
    QColor opaque(color);
    opaque.setAlphaF(1);

    QImage overlay = renderFilledImage(pixmap.size() / pixmap.devicePixelRatio(),
                                       opaque, pixmap.devicePixelRatio());
    {
        QPainter p(&overlay);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRectF({}, pixmap.size() / pixmap.devicePixelRatio()), pixmap, pixmap.rect());
    }

    if (color.alphaF() == 1)
        return UtilityFunctions::imageToPixmap(overlay);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setOpacity(color.alphaF());
    p.drawImage(QRectF({}, dest.size() / pixmap.devicePixelRatio()), overlay, overlay.rect());
    p.end();

    return dest;
}

QPixmap PaintUtils::renderPropertyColorPixmap(const QSize& size, const QString& fileName, const QPen& pen, qreal dpr)
{
    QImage dest = renderTransparentImage(size, dpr);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::NoBrush);
    p.setPen(pen);
    p.drawPixmap(QRect({}, dest.size() / dest.devicePixelRatio()), QPixmap(fileName));
    p.drawRect(QRectF{{}, size}.adjusted(0.5, 0.5, -0.5, -0.5));
    p.end();

    return UtilityFunctions::imageToPixmap(dest);
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

    return UtilityFunctions::imageToPixmap(dest);
}

void PaintUtils::drawPanelButtonBevel(QPainter* painter, const QStyleOptionButton& option)
{
    painter->save();

    const bool isSunken = option.state & QStyle::State_Sunken;

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
    midGrad.setColorAt(0, "#e4e4e4");
    midGrad.setColorAt(1, "#dedede");
    QLinearGradient buttonGrad(0, 0, 0, 1);
    buttonGrad.setCoordinateMode(QGradient::ObjectMode);
    buttonGrad.setColorAt(0, "#fdfdfd");
    buttonGrad.setColorAt(1, "#f3f3f3");
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(option.rect).adjusted(0.5, 1, -0.5, -1), 3.5, 3.5);
    painter->setPen(Qt::NoPen);
    painter->setBrush(isSunken ? midGrad : buttonGrad);
    painter->drawPath(bodyPath);
    // NOTE: QRect's bottom() and bottomLeft()... are different from QRectF, for historical reasons
    // as stated in the docs, those functions return top() + height() - 1 (QRect)

    // Draw glowing for pressed state
    if (isSunken) {
        QPainterPath glowPathUp;
        glowPathUp.addRoundedRect(QRectF(option.rect).adjusted(0, 1.5, 0, 1), 4.5, 4.5);
        QPainterPath glowPathDown;
        glowPathDown.addRoundedRect(QRectF(option.rect).adjusted(0, 2, 0, 1), 4.5, 4.5);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#ebebeb"));
        painter->drawPath(bodyPath.subtracted(glowPathDown));
        painter->setBrush(QColor("#f3f3f3"));
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

    int totalAlpha = 100 * 8;
    const int hspacing = 6 * image.devicePixelRatio();
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

    if (totalAlpha < 0)
        return false;

    if (image.width() * image.height() < 50000 * image.devicePixelRatio()) {
        for (int i = 0; i < image.width(); ++i) {
            for (int j = 0; j < image.height(); ++j)
                totalAlpha -= qAlpha(image.pixel(i, j));
            if (totalAlpha < 0)
                return false;
        }
    }

    return totalAlpha > 0;
}

static void paintTextInPlaceHolderForInvisbleItem(QPainter* painter, const QString& id,
                                                  const QRectF& boundingRect, const QPen& pen)
{
    static const qreal width = 8;
    if (boundingRect.height() > 2 * width) {
        QFont font;
        font.setStyleHint(QFont::SansSerif);
        font.setBold(true);
        font.setPixelSize(12);

        QRectF rotatedBoundingBox;
        rotatedBoundingBox.setTopLeft({width + 4, - boundingRect.width() + width});
        rotatedBoundingBox.setWidth(boundingRect.height() - 2 * width - 8);
        rotatedBoundingBox.setHeight(boundingRect.width() - 2 * width);

        QFontMetrics fm(font);
        const QString& displayText = fm.elidedText(id, Qt::ElideRight, rotatedBoundingBox.width() + 1);

        painter->rotate(90);
        painter->setFont(font);
        painter->setPen(pen);
        painter->setClipping(false);
        painter->drawText(rotatedBoundingBox, displayText, QTextOption(Qt::AlignTop));
    }
}

static void paintDecorationInPlaceHolderForInvisbleItem(QPainter* painter,
                                                        const QRectF& boundingRect,
                                                        const QBrush& brush)
{
    static const qreal width = 8;
    QPainterPath path;
    path.addRect(boundingRect);
    path.addRect(boundingRect.adjusted(width, width, -width, -width));
    painter->setClipPath(path);
    painter->setClipping(true);
    painter->fillRect(boundingRect.adjusted(1, 1, -1, -1), brush);
}

QImage PaintUtils::renderErrorControlImage(const QSizeF& size, const QString& id, qreal dpr,
                                           const QBrush& brush, const QPen& pen)
{
    QImage dest = renderBlankControlImage(QRectF(QPointF(), size), id, dpr, brush, pen);

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

QImage PaintUtils::renderBlankControlImage(const QRectF& rect, const QString& id, qreal dpr,
                                           const QBrush& brush, const QPen& pen)
{
    QImage dest = renderTransparentImage(rect.size(), dpr);
    dest.setDevicePixelRatio(dpr);

    QPainter p(&dest);
    paintDecorationInPlaceHolderForInvisbleItem(&p, rect, brush);
    paintTextInPlaceHolderForInvisbleItem(&p, id, rect, pen);
    p.end();

    return dest;
}
