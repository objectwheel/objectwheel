#ifndef ZIPPER_H
#define ZIPPER_H

#include <QString>
#include <QByteArray>

namespace Zipper
{
    void compressDir(const QString& dir, const QString& outFilename, const QString& base = QString());
    void extractZip(const QByteArray& zipData, const QString& path);
}

#endif // ZIPPER_H
