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
class QPainter;
class QStyleOption;
class QPointF;
class QIcon;
class QPalette;

namespace PaintUtils {
QImage renderFilledImage(const QSizeF& size, const QColor& fillColor, const QWidget* widget = nullptr);
QImage renderTransparentImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage renderInitialControlImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage renderInvisibleControlImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage renderErrorControlImage(const QSizeF& size, const QWidget* widget = nullptr);
QImage dpiCorrectedImage(const QImage& image, const QWidget* widget = nullptr);
QIcon renderOverlaidIcon(const QString& fileName, const QColor& color, const QWidget* widget = nullptr);
QIcon renderOverlaidIcon(const QIcon& icon, const QSize& size, const QColor& color, const QWidget* widget = nullptr);
QIcon renderButtonIcon(const QString& fileName, const QWidget* widget = nullptr);
QIcon renderOverlaidButtonIcon(const QString& fileName, const QWidget* widget = nullptr);
QIcon renderMaskedButtonIcon(const QString& fileName, const QWidget* widget = nullptr);
QPixmap renderOverlaidPixmap(const QString& fileName, const QColor& color, const QWidget* widget = nullptr);
QPixmap renderOverlaidPixmap(const QPixmap& pixmap, const QColor& color, const QWidget* widget = nullptr);
QPixmap renderMaskedPixmap(const QString& fileName, const QColor& color, const QWidget* widget = nullptr);
QPixmap renderPropertyColorPixmap(const QSize& size, const QColor& color, const QPen& pen, const QWidget* widget = nullptr);
QPalette defaultButtonPalette(bool lightTheme = true);
void drawPanelButtonBevel(QPainter* painter, const QStyleOption& option);
}

#endif // PAINTUTILS_H