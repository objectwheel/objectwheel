#ifndef PAINTUTILS_H
#define PAINTUTILS_H

class QString;
class QColor;
class QWidget;
class QPixmap;
class QSize;
class QSizeF;
class QImage;
class QPen;

namespace PaintUtils {
QImage renderFilledImage(const QSizeF& size, const QColor& fillColor, const QWidget* widget = nullptr);
QImage renderTransparentImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage renderInitialControlImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage renderInvisibleControlImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage renderErrorControlImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage renderNonGuiControlImage(const QString& url, const QSizeF& size, const QWidget* widget = nullptr);
QPixmap renderMaskedPixmap(const QString& fileName, const QColor& color, const QWidget* widget = nullptr);
QPixmap renderColoredPixmap(const QString& fileName, const QColor& color, const QWidget* widget = nullptr);
QPixmap renderColorPixmap(const QSize& size, const QColor& color, const QPen& pen, const QWidget* widget = nullptr);
}

#endif // PAINTUTILS_H