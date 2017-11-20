#ifndef ZIPPER_H
#define ZIPPER_H

#include <QString>
#include <QByteArray>

namespace Zipper
{
    bool compressDir(const QString& dir, const QString& outFilename, const QString& base = QString());
    bool extractZip(const QByteArray& zipData, const QString& path);
}

#endif // ZIPPER_H
