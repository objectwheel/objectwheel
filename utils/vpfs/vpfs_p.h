#ifndef VPFS_P_H
#define VPFS_P_H

#include <private/qabstractfileengine_p.h>
#include <vpfs.h>

class VpfsFileEnginePrivate : public QAbstractFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(VpfsFileEngine)
private:
    uchar* map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
    bool unmap(uchar* ptr);
    void uncompress() const;
    qint64 offset;
    Vpfs resource;
    mutable QByteArray uncompressed;
protected:
    VpfsFileEnginePrivate() : offset(0) { }
};

#endif // VPFS_P_H
