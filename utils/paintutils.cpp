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

QPixmap PaintUtils::pixmap(const QString& imagePath, const QSize& size, const QWidget* widget,
                           QIcon::Mode mode, QIcon::State state)
{
    return pixmap(QIcon(imagePath), size, widget, mode, state);
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

    QImage overlay = renderFilledImage(pixmap.size() / pixmap.devicePixelRatioF(),
                                       opaque, pixmap.devicePixelRatioF());
    {
        QPainter p(&overlay);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRectF({}, pixmap.size() / pixmap.devicePixelRatioF()), pixmap, pixmap.rect());
    }

    if (color.alphaF() == 1)
        return UtilityFunctions::imageToPixmap(overlay);

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setOpacity(color.alphaF());
    p.drawImage(QRectF({}, dest.size() / pixmap.devicePixelRatioF()), overlay, overlay.rect());
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
