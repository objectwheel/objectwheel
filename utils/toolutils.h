#ifndef TOOLUTILS_H
#define TOOLUTILS_H

class QString;
class QPixmap;

namespace ToolUtils {

QString toolIconPath(const QString& module);
QString toolCetegory(const QString& module);
int toolCetegoryIndex(const QString& category);

} // ToolUtils

#endif // TOOLUTILS_H
