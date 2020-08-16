#ifndef TOOLUTILS_H
#define TOOLUTILS_H

#include <utils_global.h>

namespace ToolUtils {

UTILS_EXPORT QString toolIconPath(const QString& module);
UTILS_EXPORT QString toolCetegory(const QString& module);
UTILS_EXPORT int toolCetegoryIndex(const QString& category);

} // ToolUtils

#endif // TOOLUTILS_H
