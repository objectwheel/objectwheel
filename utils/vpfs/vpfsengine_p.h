#ifndef VPFSENGINE_P_H
#define VPFSENGINE_P_H

#include <private/qabstractfileengine_p.h>
#include <vpfs.h>

class VpfsEnginePrivate : public QAbstractFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(VpfsEngine)
private:
    uchar* map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
    bool unmap(uchar* ptr);
    void uncompress() const;
    qint64 offset;
    Vpfs resource;
    mutable QByteArray uncompressed;
protected:
    VpfsEnginePrivate() : offset(0) { }
};

#endif // VPFSENGINE_P_H
