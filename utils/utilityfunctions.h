#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <Qt>

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
QWindow* window(const QWidget* w);
void centralizeWidget(QWidget* widget);
void adjustFontPixelSize(QWidget* w, int advance);
}

#endif // UTILITYFUNCTIONS_H