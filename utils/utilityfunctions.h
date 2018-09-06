#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <Qt>

class QTextDocument;
class QWidget;
class QUrl;
class QModelIndex;
class QTreeView;

namespace UtilityFunctions {
void registerGlobalPath(const QString& projectDir);
void trimCommentsAndStrings(QTextDocument* document);
QWidget* createSpacerWidget(Qt::Orientation orientation);
void copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent);
void expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex);
}

#endif // UTILITYFUNCTIONS_H