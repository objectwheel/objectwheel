#ifndef TOOLUTILS_H
#define TOOLUTILS_H

class QString;
class QPixmap;

namespace ToolUtils {

QPixmap toolIcon(const QString& controlDir, double dpr);
QString toolName(const QString& controlDir);
QString toolCetegory(const QString& controlDir);

} // ToolUtils

#endif // TOOLUTILS_H
