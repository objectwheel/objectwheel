#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include <QIcon>

class QStyleOption;
class QStyleOptionButton;

namespace PaintUtils {

bool isBlankImage(const QImage& image);
void drawPanelButtonBevel(QPainter* painter, const QStyleOptionButton& option);
QPixmap pixmap(const QString& imagePath, const QSize& size, const QWidget* widget = nullptr,
               QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
QPixmap pixmap(const QIcon& icon, const QSize& size, const QWidget* widget = nullptr,
               QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
QIcon renderButtonIcon(const QString& imagePath, const QSize& size, const QWidget* widget);
QPixmap renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, qreal dpr);
QPixmap renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color);
QPixmap renderOverlaidPixmap(const QString& fileName, const QColor& color,
                             const QSize& size, const QWidget* widget);
QImage renderErrorControlImage(const QSizeF& size, const QString& id,
                               const QBrush& brush, const QPen& pen, const QWidget* widget);
QImage renderBlankControlImage(const QRectF& rect, const QString& id, qreal dpr,
                               const QBrush& brush, const QPen& pen);
} // PaintUtils

#endif // PAINTUTILS_H