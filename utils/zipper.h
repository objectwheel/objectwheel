#ifndef ZIPPER_H
#define ZIPPER_H

#include <utils_global.h>
#include <QString>
#include <QByteArray>

namespace Zipper
{
    UTILS_EXPORT bool compressDir(const QString& dir, const QString& outFilename, const QString& base = QString());
    UTILS_EXPORT bool extractZip(const QByteArray& zipData, const QString& path);
}

#endif // ZIPPER_H
