#ifndef PAINTUTILS_H
#define PAINTUTILS_H

class QString;
class QColor;
class QWidget;
class QPixmap;

namespace PaintUtils {
QPixmap maskedPixmap(const QString& fileName, const QColor& color, const QWidget* widget);
}

#endif // PAINTUTILS_H