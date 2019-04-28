#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include <QtGlobal>

class QStringList;

namespace FileSystemUtils {

QStringList searchFiles(const QString& fileName, const QString& dirPath);

bool copy(const QString& fromPath, const QString& toDir, bool content = false,
          bool fixPermissions = false);

bool makeFile(const QString& filePath);

qint64 directorySize(const QString& dirPath);

} // FileSystemUtils

#endif // FILESYSTEMUTILS_H
