#ifndef TOOLUTILS_H
#define TOOLUTILS_H

class QString;
class QPixmap;

namespace ToolUtils {

QString toolName(const QString& controlDir);
QString toolIconPath(const QString& controlDir);
QString toolCetegory(const QString& controlDir);
int toolCetegoryIndex(const QString& category);

} // ToolUtils

#endif // TOOLUTILS_H
