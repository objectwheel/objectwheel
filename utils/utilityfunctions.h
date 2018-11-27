#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <Qt>
#include <QFont>

#include <previewresult.h>

class QTextDocument;
class QWidget;
class QUrl;
class QModelIndex;
class QTreeView;
class QWindow;
class QQmlEngine;
class QAbstractButton;
class QAbstractItemView;
class QQmlError;
class QComboBox;
class QMarginsF;

namespace UtilityFunctions {
void trimCommentsAndStrings(QTextDocument* document);
void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent);
void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex);
void centralizeWidget(QWidget* widget);
void adjustFontWeight(QWidget* widget, QFont::Weight weight);
void adjustFontPixelSize(QWidget* widget, int advance);
bool hasHover(const QWidget* widget);
QWidget* createSpacingWidget(const QSize& size);
QWidget* createSpacerWidget(Qt::Orientation orientation);
QWidget* createSeparatorWidget(Qt::Orientation orientation);
QWindow* window(const QWidget* widget);
QRectF verticalAlignedRect(const QSizeF& size, const QRectF& rect, qreal left = 0);
QRectF horizontalAlignedRect(const QSizeF& size, const QRectF& rect, qreal top = 0);
QPixmap pixmap(QAbstractButton* button, const QIcon& icon, const QSizeF& size);
QPixmap scaled(const QPixmap& pixmap, const QSize& size);
QIcon iconForQmlError(const QQmlError& error, const QAbstractItemView* view);
QPoint centerPos(const QSize& size);
QString increasedNumberedText(const QString& text, bool addSpace, bool trim);
bool comboContainsWord(QComboBox* comboBox, const QString& word);
QRectF getGeometryFromProperties(const QList<PropertyNode>& properties);
QMarginsF getMarginsFromProperties(const QList<PropertyNode>& properties);
void putMarginsToProperties(QMap<QString, QVariant>& properties, const QMarginsF& margins);
QVariant getProperty(const QString& property, const QList<PropertyNode>& properties);
Enum getEnum(const QString& name, const QList<PropertyNode>& properties);
}

#endif // UTILITYFUNCTIONS_H