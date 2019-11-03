#ifndef TOOLUTILS_H
#define TOOLUTILS_H

class QString;
class QPixmap;

namespace ToolUtils {

QString toolNameFromModule(const QString& module);
QString toolIconPathFromModule(const QString& module);
QString toolCetegoryFromModule(const QString& module);
int toolCetegoryIndex(const QString& category);

} // ToolUtils

#endif // TOOLUTILS_H
