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

namespace UtilityFunctions {
void registerOfflineStorage();
void registerGlobalPath(const QString& projectDirectory);
void trimCommentsAndStrings(QTextDocument* document);
void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent);
void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex);
QWidget* createSpacerWidget(Qt::Orientation orientation);
QWindow* window(const QWidget* widget);
void centralizeWidget(QWidget* widget);
void adjustFontWeight(QWidget* widget, QFont::Weight weight);
void adjustFontPixelSize(QWidget* widget, int advance);
bool hasHover(const QWidget* widget);
}

#endif // UTILITYFUNCTIONS_H