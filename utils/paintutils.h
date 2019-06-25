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

namespace PaintUtils {

QImage renderFilledImage(const QSizeF& size, const QColor& fillColor, qreal dpr);
QImage renderTransparentImage(const QSizeF& size, qreal dpr);
QImage renderInitialControlImage(const QSizeF& size, qreal dpr);
QImage renderInvisibleControlImage(const QSizeF& size, qreal dpr);
QImage renderErrorControlImage(const QSizeF& size, qreal dpr);
QImage renderNonGuiControlImage(const QPixmap& pixmap, const QSizeF& size, qreal dpr);
QIcon renderOverlaidIcon(const QString& fileName, const QColor& color, qreal dpr);
QIcon renderOverlaidIcon(const QIcon& icon, const QSize& size, const QColor& color, const QWidget* widget);
QIcon renderButtonIcon(const QString& fileName, qreal dpr);
QIcon renderToolButtonIcon(const QString& fileName, qreal dpr);
QIcon renderOverlaidButtonIcon(const QString& fileName, const QWidget* widget);
QPixmap renderOverlaidPixmap(const QString& fileName, const QColor& color, qreal dpr);
QPixmap renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color, qreal dpr);
QPixmap renderMaskedPixmap(const QString& fileName, const QColor& color, qreal dpr);
QPixmap renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, qreal dpr);
void drawPanelButtonBevel(QPainter* painter, const QStyleOptionButton& option, bool downWhenChecked = true);
void drawSearchEditBevel(QPainter* painter, const QStyleOption& option);
bool isBlankImage(const QImage& image);

} // PaintUtils

#endif // PAINTUTILS_H