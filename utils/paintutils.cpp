#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QImage>
#include <QBitmap>
#include <QWidget>
#include <QApplication>
#include <QStyleOption>
#include <QPalette>

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
    QIcon icon;
    icon.addPixmap(QPixmap(fileName), QIcon::Normal);
    icon.addPixmap(renderOverlaidPixmap(fileName, "#20000000", widget), QIcon::Active);
    return icon;
}

QIcon PaintUtils::renderOverlaidButtonIcon(const QString& fileName, const QWidget* widget)
{
    QIcon icon;
    QColor up = widget->palette().buttonText().color();
    QColor down = widget->palette().buttonText().color().darker();
    icon.addPixmap(renderOverlaidPixmap(fileName, up, widget), QIcon::Normal);
    icon.addPixmap(renderOverlaidPixmap(fileName, down, widget), QIcon::Active);
    return icon;
}

QIcon PaintUtils::renderMaskedButtonIcon(const QString& fileName, const QWidget* widget)
{
    QIcon icon;
    QColor up = widget->palette().buttonText().color();
    QColor down = widget->palette().buttonText().color().darker();
    icon.addPixmap(renderMaskedPixmap(fileName, up, widget), QIcon::Normal);
    icon.addPixmap(renderMaskedPixmap(fileName, down, widget), QIcon::Active);
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

    // Draw drop shadow
    QPainterPath dropShadowPath;
    dropShadowPath.addRect(3.5, option.rect.bottom() + 0.5, option.rect.width() - 7, 1);
    painter->setPen(Qt::NoPen);
    painter->setBrush(option.palette.shadow());
    painter->drawPath(dropShadowPath);

    // Draw shadow
    QPainterPath shadowPath;
    shadowPath.addRoundedRect(QRectF(option.rect).adjusted(0, 0.5, 0, -0.5), 4.25, 4.25);
    painter->setPen(Qt::NoPen);
    painter->setBrush(option.palette.dark());
    painter->drawPath(shadowPath);

    // Draw body
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(QRectF(option.rect).adjusted(0.5, 1, -0.5, -1), 3.65, 3.65);
    painter->setPen(Qt::NoPen);
    painter->setBrush(down ? option.palette.mid() : option.palette.button());
    painter->drawPath(bodyPath);
    // NOTE: QRect's bottom() and bottomLeft()... are different from QRectF, for historical reasons
    // as stated in the docs, those functions return top() + height() - 1 (QRect)

    // Draw glowing for pressed state
    if (down) {
        QPainterPath glowPath;
        glowPath.addRoundedRect(QRectF(option.rect).adjusted(0.5, 2, -0.5, 1), 3.65, 3.65);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.midlight());
        painter->drawPath(bodyPath.subtracted(glowPath));
    }

    painter->restore();
}

/*!
    QPalette::Light     :  Button's frame rect color (not used)
    QPalette::Midlight  :  Button's glowing color (when pressed)
    QPalette::Button    :  Button's normal body color
    QPalette::Mid       :  Button's pressed body color
    QPalette::Dark      :  Button's base and surrounding border's color
    QPalette::Shadow    :  Button's drop shadow color
*/
QPalette PaintUtils::defaultButtonPalette(bool lightTheme)
{
    QPalette palette(qApp->palette());
    QLinearGradient shadowGrad({0.0, 0.5}, {1.0, 0.5});
    shadowGrad.setCoordinateMode(QGradient::ObjectMode);
    if (lightTheme) {
        shadowGrad.setColorAt(0, "#12202020");
        shadowGrad.setColorAt(0.05, "#10202020");
        shadowGrad.setColorAt(0.5, "#10202020");
        shadowGrad.setColorAt(0.95, "#10202020");
        shadowGrad.setColorAt(1, "#12202020");
    } else {
        shadowGrad.setColorAt(0, "#12202020");
        shadowGrad.setColorAt(0.05, "#10202020");
        shadowGrad.setColorAt(0.5, "#10202020");
        shadowGrad.setColorAt(0.95, "#10202020");
        shadowGrad.setColorAt(1, "#12202020");
    }
    palette.setBrush(QPalette::Shadow, shadowGrad);

    QLinearGradient darkGrad({0.0, 0.0}, {0.0, 1.0});
    darkGrad.setCoordinateMode(QGradient::ObjectMode);
    if (lightTheme) {
        darkGrad.setColorAt(0.85, "#20303030");
        darkGrad.setColorAt(1, "#3d000000");
    } else {
        darkGrad.setColorAt(0.85, "#20303030");
        darkGrad.setColorAt(1, "#3d000000");
    }
    palette.setBrush(QPalette::Dark, darkGrad);

    QLinearGradient midGrad({0.0, 0.0}, {0.0, 1.0});
    midGrad.setCoordinateMode(QGradient::ObjectMode);
    if (lightTheme) {
        midGrad.setColorAt(0, "#e7e7e7");
        midGrad.setColorAt(1, "#e1e1e1");
    } else {
        midGrad.setColorAt(0, "#e7e7e7");
        midGrad.setColorAt(1, "#e1e1e1");
    }
    palette.setBrush(QPalette::Mid, midGrad);

    QLinearGradient buttonGrad({0.0, 0.0}, {0.0, 1.0});
    buttonGrad.setCoordinateMode(QGradient::ObjectMode);
    if (lightTheme) {
        buttonGrad.setColorAt(0, "#fefefe");
        buttonGrad.setColorAt(1, "#f7f7f7");
    } else {
        buttonGrad.setColorAt(0, "#637280");
        buttonGrad.setColorAt(1, "#5b6975");
    }
    palette.setBrush(QPalette::Button, buttonGrad);

    QLinearGradient midlightGrad({0.5, 0.0}, {0.5, 1.0});
    midlightGrad.setCoordinateMode(QGradient::ObjectMode);
    if (lightTheme) {
        midlightGrad.setColorAt(0, "#f4f4f4");
        midlightGrad.setColorAt(0.1, "#ededed");
    } else {
        midlightGrad.setColorAt(0, "#f4f4f4");
        midlightGrad.setColorAt(0.1, "#ededed");
    }
    palette.setBrush(QPalette::Midlight, midlightGrad);

    if (lightTheme)
        palette.setColor(QPalette::ButtonText, "#555555");
    else
        palette.setColor(QPalette::ButtonText, "#555555");

    return palette;
}
