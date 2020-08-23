#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include <utils_global.h>
#include <QIcon>

class QStyleOption;
class QTextOption;

namespace PaintUtils {

UTILS_EXPORT bool isBlankImage(const QImage& image);
UTILS_EXPORT QPixmap pixmap(const QString& imagePath, const QSize& size, const QWidget* widget = nullptr,
                            QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
UTILS_EXPORT QPixmap pixmap(const QIcon& icon, const QSize& size, const QWidget* widget = nullptr,
                            QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
UTILS_EXPORT QIcon renderButtonIcon(const QString& imagePath, const QSize& size, const QWidget* widget);
UTILS_EXPORT QPixmap renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, qreal dpr);
UTILS_EXPORT QPixmap renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color);
UTILS_EXPORT QPixmap renderOverlaidPixmap(const QString& fileName, const QColor& color,
                                          const QSize& size, const QWidget* widget);
UTILS_EXPORT QImage renderErrorControlImage(const QSizeF& size, const QString& id,
                                            const QBrush& brush, const QPen& pen, const QWidget* widget);
UTILS_EXPORT QImage renderBlankControlImage(const QRectF& rect, const QString& id, qreal dpr,
                                            const QBrush& brush, const QPen& pen);
UTILS_EXPORT QColor disabledColor(const QColor& color);
UTILS_EXPORT void drawHtml(QPainter* painter, const QString& title, const QPointF& offset,
                           const QFont& font, const QColor& color, const QTextOption& option);
} // PaintUtils

#endif // PAINTUTILS_H
