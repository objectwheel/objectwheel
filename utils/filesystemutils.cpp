#include <filesystemutils.h>
#include <QDir>

namespace FileSystemUtils {

namespace Internal {

bool copyDir(QString fromDir, QString toDir, bool fixPermissions)
{
    for (const QString& fileName : QDir(fromDir).entryList(QDir::Files)) {
        const QString& from = fromDir + QStringLiteral("/") + fileName;
        const QString& to = toDir + QStringLiteral("/") + fileName;
        if (!QFile::copy(from, to))
            return false;
        if (fixPermissions)
            QFile::setPermissions(to, QFile::WriteUser | QFile::ReadUser);
    }
    for (const QString& dirName : QDir(fromDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& from = fromDir + QStringLiteral("/") + dirName;
        const QString& to = toDir + QStringLiteral("/") + dirName;
        if (!QDir(to).mkpath(QStringLiteral(".")))
            return false;
        if (!copyDir(from, to, fixPermissions))
            return false;
    }
    return true;
}

} // Internal

bool copy(const QString& fromPath, const QString& toDir, bool content, bool fixPermissions)
{
    if (fromPath == toDir)
        return true;

    if (!QFileInfo::exists(fromPath) || !QFileInfo::exists(toDir))
        return false;

    const QString& dest = toDir + QStringLiteral("/") + QFileInfo(fromPath).fileName();

    if (QFileInfo(fromPath).isDir()) {
        if (content) {
            return Internal::copyDir(fromPath, toDir, fixPermissions);
        } else {
            if (!QDir(dest).mkpath(QStringLiteral(".")))
                return false;
            return Internal::copyDir(fromPath, dest, fixPermissions);
        }
    } else {
        if (!QFile::copy(fromPath, dest))
            return false;
        if (fixPermissions)
            QFile::setPermissions(dest, QFile::WriteUser | QFile::ReadUser);
        return true;
    }
}

bool makeFile(const QString& filePath)
{
    QFileInfo info(filePath);
    if (info.fileName().isEmpty())
        return false;
    if (info.exists())
        return true;
    if (!info.dir().mkpath(QStringLiteral(".")))
        return false;
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly))
        return false;
    return true;
}

qint64 directorySize(const QString& dirPath)
{
    qint64 size = 0;
    for (const QString& fileName : QDir(dirPath).entryList(QDir::Files | QDir::System | QDir::Hidden
                                                           | QDir::NoSymLinks)) {
        size += QFileInfo(dirPath + QStringLiteral("/") + fileName).size();
    }
    for (const QString& dirName : QDir(dirPath).entryList(QDir::AllDirs | QDir::System
                                                          | QDir::Hidden | QDir::NoSymLinks
                                                          | QDir::NoDotAndDotDot)) {
        size += directorySize(dirPath + QStringLiteral("/") + dirName);
    }
    return size;
}

QStringList searchFiles(const QString& filename, const QString& dirPath)
{
    QStringList found;
    for (const QString& fileName : QDir(dirPath).entryList(QDir::Files)) {
        if (fileName == filename)
            found.append(dirPath + QStringLiteral("/") + fileName);
    }
    for (const QString& dirName : QDir(dirPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        found.append(searchFiles(filename, dirPath + QStringLiteral("/") + dirName));
    return found;
}

QStringList searchFilesBySuffix(const QStringList& suffixes, const QString& dirPath)
{
    QStringList found;
    for (const QString& fileName : QDir(dirPath).entryList(QDir::Files)) {
        if (suffixes.contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive))
            found.append(dirPath + QStringLiteral("/") + fileName);
    }
    for (const QString& dirName : QDir(dirPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        found.append(searchFilesBySuffix(suffixes, dirPath + QStringLiteral("/") + dirName));
    return found;
}

} // FileSystemUtils
