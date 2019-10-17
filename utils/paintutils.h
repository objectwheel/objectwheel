#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include <QtGlobal>
#include <QPen>

class QString;
class QColor;
class QWidget;
class QPixmap;
class QSize;
class QSizeF;
class QImage;
class QPainter;
class QStyleOption;
class QStyleOptionButton;
class QPointF;
class QIcon;
class QPalette;
class QByteArray;
class QRectF;

namespace PaintUtils {
void drawPanelButtonBevel(QPainter* painter, const QStyleOptionButton& option);
void drawSearchEditBevel(QPainter* painter, const QStyleOption& option);
bool isBlankImage(const QImage& image);
QIcon renderButtonIcon(const QString& fileName, const QPalette& palette);
QPixmap renderOverlaidPixmap(const QString& fileName, const QColor& color, qreal dpr);
QPixmap renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color);
QPixmap renderPropertyColorPixmap(const QSize& size, const QString& fileName, const QPen& pen, qreal dpr);
QPixmap renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, qreal dpr);
QImage renderFilledImage(const QSizeF& size, const QColor& fillColor, qreal dpr);
QImage renderTransparentImage(const QSizeF& size, qreal dpr);
QImage renderNonGuiControlImage(const QString& imagePath, const QSizeF& size, void* widget);
QImage renderErrorControlImage(const QSizeF& size, const QString& id,
                               const QBrush& brush,
                               const QPen& pen, void* widget);
QImage renderBlankControlImage(const QRectF& rect, const QString& id, qreal dpr,
                               const QBrush& brush,
                               const QPen& pen);
} // PaintUtils

#endif // PAINTUTILS_H