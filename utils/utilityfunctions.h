#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <Qt>

class QTextDocument;
class QWidget;

namespace UtilityFunctions {
void registerGlobalPath(const QString& projectDir);
void trimCommentsAndStrings(QTextDocument* document);
QWidget* createSpacerWidget(Qt::Orientation orientation);
}

#endif // UTILITYFUNCTIONS_H