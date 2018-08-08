#ifndef PAINTUTILS_H
#define PAINTUTILS_H

class QString;
class QColor;
class QWidget;
class QPixmap;
class QSize;
class QSizeF;
class QImage;

namespace PaintUtils {
QPixmap maskedPixmap(const QString& fileName, const QColor& color, const QWidget* widget = nullptr);
QPixmap colorToPixmap(const QSize& size, const QColor& color, const QWidget* widget = nullptr,
                     bool hasBorder = true);
QImage controlErrorImage(const QSizeF& size, double dpr);
}

#endif // PAINTUTILS_H