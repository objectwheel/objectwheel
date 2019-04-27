#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

class QString;
class QStringList;

namespace FileSystemUtils {

QStringList searchFiles(const QString& fileName, const QString& dir);

bool copy(const QString& fromPath, const QString& toDir, bool content = false,
          bool fixPermissions = false);

} // FileSystemUtils

#endif // FILESYSTEMUTILS_H
