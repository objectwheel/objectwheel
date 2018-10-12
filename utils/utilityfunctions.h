#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <Qt>
#include <QFont>

class QTextDocument;
class QWidget;
class QUrl;
class QModelIndex;
class QTreeView;
class QWindow;
class QQmlEngine;
class QAbstractButton;

namespace UtilityFunctions {
void registerOfflineStorage();
void registerGlobalPath(const QString& projectDirectory);
void trimCommentsAndStrings(QTextDocument* document);
void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent);
void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex);
void centralizeWidget(QWidget* widget);
void adjustFontWeight(QWidget* widget, QFont::Weight weight);
void adjustFontPixelSize(QWidget* widget, int advance);
bool hasHover(const QWidget* widget);
QWidget* createSpacingWidget(const QSize& size);
QWidget* createSpacerWidget(Qt::Orientation orientation);
QWindow* window(const QWidget* widget);
QRectF verticalAlignedRect(const QSizeF& size, const QRectF& rect, qreal left = 0);
QRectF horizontalAlignedRect(const QSizeF& size, const QRectF& rect, qreal top = 0);
QPixmap pixmap(QAbstractButton* button, const QIcon& icon, const QSizeF& size);
QPixmap scaled(const QPixmap& pixmap, const QSize& size);
}

#endif // UTILITYFUNCTIONS_H