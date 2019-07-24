#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include <QtGlobal>

class QString;
class QColor;
class QWidget;
class QPixmap;
class QSize;
class QSizeF;
class QImage;
class QPen;
class QPainter;
class QStyleOption;
class QStyleOptionButton;
class QPointF;
class QIcon;
class QPalette;
class QByteArray;
class QRectF;

namespace PaintUtils {

QImage renderFilledImage(const QSizeF& size, const QColor& fillColor, qreal dpr);
QImage renderTransparentImage(const QSizeF& size, qreal dpr);
QImage renderInitialControlImage(const QSizeF& size, qreal dpr);
QImage renderErrorControlImage(const QSizeF& size, qreal dpr);
QImage renderNonGuiControlImage(const QString& imagePath, const QSizeF& size, qreal dpr);
QImage renderBlankControlImage(const QRectF& frame, const QRectF& rect, const QString& id, qreal dpr);
QIcon renderItemIcon(const QString& fileName, const QPalette& palette);
QIcon renderButtonIcon(const QString& fileName, const QPalette& palette);
QIcon renderModeButtonIcon(const QString& fileName);
QPixmap renderOverlaidPixmap(const QString& fileName, const QColor& color, qreal dpr);
QPixmap renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color);
QPixmap renderPropertyColorPixmap(const QSize& size, const QString& fileName, const QPen& pen, qreal dpr);
QPixmap renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, qreal dpr);
void drawPanelButtonBevel(QPainter* painter, const QStyleOptionButton& option);
void drawSearchEditBevel(QPainter* painter, const QStyleOption& option);
bool isBlankImage(const QImage& image);

} // PaintUtils

#endif // PAINTUTILS_H