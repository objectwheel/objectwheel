#include <filesystemutils.h>
#include <QDir>

namespace FileSystemUtils {

namespace Internal {

bool copyDir(QString fromDir, QString toDir, bool fixPermissions)
{
    for (const QString& fileName : QDir(fromDir).entryList(QDir::Files)) {
        const QString& from = fromDir + '/' + fileName;
        const QString& to = toDir + '/' + fileName;
        if (!QFile::copy(from, to))
            return false;
        if (fixPermissions)
            QFile::setPermissions(to, QFile::WriteUser | QFile::ReadUser);
    }
    for (const QString& dirName : QDir(fromDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& from = fromDir + '/' + dirName;
        const QString& to = toDir + '/' + dirName;
        if (!QDir(to).mkpath("."))
            return false;
        if (!copyDir(from, to, fixPermissions))
            return false;
    }
    return true;
}

} // Internal

QStringList searchFiles(const QString& filename, const QString& dir)
{
    QStringList found;
    for (const QString& fileName : QDir(dir).entryList(QDir::Files)) {
        if (fileName == filename)
            found << dir + '/' + fileName;
    }
    for (const QString& dirName : QDir(dir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        found << searchFiles(filename, dir + '/' + dirName);
    return found;
}

bool copy(const QString& fromPath, const QString& toDir, bool content, bool fixPermissions)
{
    if (fromPath == toDir)
        return true;

    if (!QFileInfo::exists(fromPath) || !QFileInfo::exists(toDir))
        return false;

    const QString& dest = toDir + '/' + QFileInfo(fromPath).fileName();

    if (QFileInfo(fromPath).isDir()) {
        if (content) {
            return Internal::copyDir(fromPath, toDir, fixPermissions);
        } else {
            if (!QDir(dest).mkpath("."))
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

} // FileSystemUtils