#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

class QString;
class QTextDocument;

namespace UtilityFunctions {
void registerGlobalPath(const QString& projectDir);
void trimCommentsAndStrings(QTextDocument* document);
}

#endif // UTILITYFUNCTIONS_H