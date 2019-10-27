#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QApplication>
#include <QStyleOption>
#include <QWindow>
#include <QScreen>

static QImage renderFilledImage(const QSizeF& size, const QColor& fillColor, qreal dpr)
{
    QImage dest((size * dpr).toSize(), QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(dpr);
    dest.fill(fillColor);
    return dest;
}

static QImage renderTransparentImage(const QSizeF& size, qreal dpr)
{
    return renderFilledImage(size, Qt::transparent, dpr);
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

static void paintDecorationInPlaceHolderForInvisbleItem(QPainter* painter, const QRectF& boundingRect,
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

QPixmap PaintUtils::pixmap(const QString& imagePath, const QSize& size, const QWidget* widget,
                           QIcon::Mode mode, QIcon::State state)
{
    const QIcon icon(imagePath);
    if (QWindow* window = widget ? UtilityFunctions::window(widget) : nullptr)
        return icon.pixmap(window, size, mode, state);
    QWindow fakeWindow; // This makes QIcon to use primary screen's dpr
    return icon.pixmap(&fakeWindow, size, mode, state);
}

QPixmap PaintUtils::pixmap(const QIcon& icon, const QSize& size, const QWidget* widget,
                           QIcon::Mode mode, QIcon::State state)
{
    if (QWindow* window = widget ? UtilityFunctions::window(widget) : nullptr)
        return icon.pixmap(window, size, mode, state);
    QWindow fakeWindow; // This makes QIcon to use primary screen's dpr
    return icon.pixmap(&fakeWindow, size, mode, state);
}

QIcon PaintUtils::renderButtonIcon(const QString& imagePath, const QSize& size, const QWidget* widget)
{
    QIcon icon;
    QColor up = widget ? widget->palette().color(QPalette::Active, QPalette::ButtonText) : Qt::black;
    QColor down = widget ? widget->palette().color(QPalette::Active, QPalette::Highlight) : Qt::white;
    icon.addPixmap(renderOverlaidPixmap(imagePath, up, size, widget), QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderOverlaidPixmap(imagePath, down, size, widget), QIcon::Normal, QIcon::On);
    return icon;
}

QPixmap PaintUtils::renderPropertyColorPixmap(const QSize& size, const QColor& color,
                                              const QPen& pen, qreal dpr)
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

QPixmap PaintUtils::renderOverlaidPixmap(const QString& fileName, const QColor& color,
                                         const QSize& size, const QWidget* widget)
{
    return renderOverlaidPixmap(pixmap(fileName, size, widget), color);
}

QImage PaintUtils::renderErrorControlImage(const QSizeF& size, const QString& id,
                                           const QBrush& brush, const QPen& pen, const QWidget* widget)
{
    const qreal dpr = widget ? widget->devicePixelRatio() : QApplication::primaryScreen()->devicePixelRatio();
    const QSize iconSize(16, 16);

    QImage dest = renderBlankControlImage(QRectF(QPointF(), size), id, dpr, brush, pen);
    QImage source(pixmap(":/images/output/issue.svg", iconSize, widget).toImage());
    source.setDevicePixelRatio(dpr);

    QRectF destRect{{}, size};
    QRectF sourceRect{{}, iconSize};
    sourceRect.moveCenter(destRect.center());

    QPainter p(&dest);
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

void PaintUtils::drawPushButtonBevel(QPainter* painter, const QStyleOption* option)
{
    const QRectF rect = option->rect;
    const bool hasHover = option->state & QStyle::State_MouseOver;
    const bool isEnabled = option->state & QStyle::State_Enabled;
    const bool isDown = (option->state & QStyle::State_Sunken) || (option->state & QStyle::State_On);

    bool hasMenu = false;
    bool isFlat = false;
    bool isDefault = false;

    if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        hasMenu = button->features & QStyleOptionButton::HasMenu;
        isFlat = button->features & QStyleOptionButton::Flat;
        isDefault = (button->features & QStyleOptionButton::DefaultButton) && isEnabled;
    }

    painter->save();

    if (!isFlat || isDown) {
        // Draw shadows
        QLinearGradient shadowGrad(0, 0, 0, 1);
        shadowGrad.setCoordinateMode(QGradient::ObjectMode);
        shadowGrad.setColorAt(0.0, "#07000000");
        shadowGrad.setColorAt(0.1, "#07000000");
        shadowGrad.setColorAt(0.9, "#07000000");
        shadowGrad.setColorAt(1.0, "#15000000");
        painter->setPen(Qt::NoPen);
        painter->setBrush(shadowGrad);
        painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), 4, 4);
        painter->setBrush(QColor("#09000000"));
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), 4, 4);

        // Draw border
        QLinearGradient borderGrad(0, 0, 0, 1);
        borderGrad.setCoordinateMode(QGradient::ObjectMode);
        borderGrad.setColorAt(0.0, isDown ? "#3280f7" : (isDefault ? "#5094f7" : "#d8d8d8"));
        borderGrad.setColorAt(0.1, isDown ? "#2e7bf3" : (isDefault ? "#468ef8" : "#d0d0d0"));
        borderGrad.setColorAt(0.9, isDown ? "#1a5fda" : (isDefault ? "#196dfb" : "#d0d0d0"));
        borderGrad.setColorAt(1.0, isDown ? "#1659d5" : (isDefault ? "#1367fb" : "#bcbcbc"));
        painter->setBrush(borderGrad);
        painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), 3.5, 3.5);

        // Draw body
        QLinearGradient bodyGrad(0, 0, 0, 1);
        bodyGrad.setCoordinateMode(QGradient::ObjectMode);
        bodyGrad.setColorAt(0.0, isDown ? "#5496f9" : (isDefault ? "#6fa7f8" : "white"));
        bodyGrad.setColorAt(0.9, isDown ? "#1c65dd" : (isDefault ? "#176ffb" : "white"));
        bodyGrad.setColorAt(1.0, isDown ? "#1c65dd" : (isDefault ? "#176ffb" : "white"));
        painter->setBrush(bodyGrad);
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5), 3, 3);
    }

    if (hasMenu && (hasHover || isDown)) {
        if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            int mbi = qApp->style()->pixelMetric(QStyle::PM_MenuButtonIndicator, button);
            QStyleOptionButton newBtn = *button;
            newBtn.rect = QRect(rect.right() - mbi, rect.bottom() - mbi, mbi, mbi);
            qApp->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, painter);
        }
    }

    painter->restore();
}
