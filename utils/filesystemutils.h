#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include <utils_global.h>
#include <QtGlobal>

class QStringList;

namespace FileSystemUtils {

UTILS_EXPORT bool copy(const QString& fromPath, const QString& toDir, bool content = false,
                       bool fixPermissions = false);

UTILS_EXPORT bool makeFile(const QString& filePath);

UTILS_EXPORT qint64 directorySize(const QString& dirPath);

UTILS_EXPORT QStringList searchFiles(const QString& fileName, const QString& dirPath);
UTILS_EXPORT QStringList searchFilesBySuffix(const QStringList& suffixes, const QString& dirPath);

} // FileSystemUtils

#endif // FILESYSTEMUTILS_H
