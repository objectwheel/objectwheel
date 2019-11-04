#ifndef TOOLUTILS_H
#define TOOLUTILS_H

class QString;
class QPixmap;

namespace ToolUtils {

int toolMajor(const QString& module);
int toolMinor(const QString& module);
QString toolQualifiedName(const QString& module);
QString toolName(const QString& module);
QString toolIconPath(const QString& module);
QString toolCetegory(const QString& module);
int toolCetegoryIndex(const QString& category);

} // ToolUtils

#endif // TOOLUTILS_H
