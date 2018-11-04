#include <vpfs.h>
#include <vpfs_p.h>
#include <vpfs_iterator_p.h>

#include <private/qsystemerror_p.h>

#ifdef Q_OS_UNIX
# include <private/qcore_unix_p.h>
#endif

#include <QDateTime>

bool VpfsFileEngine::mkdir(const QString &, bool) const
{
    return false;
}

bool VpfsFileEngine::rmdir(const QString &, bool) const
{
    return false;
}

bool VpfsFileEngine::setSize(qint64)
{
    return false;
}

QStringList VpfsFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    return QAbstractFileEngine::entryList(filters, filterNames);
}

bool VpfsFileEngine::caseSensitive() const
{
    return true;
}

VpfsFileEngine::VpfsFileEngine(const QString &file) :
    QAbstractFileEngine(*new VpfsFileEnginePrivate)
{
    Q_D(VpfsFileEngine);
    d->resource.setFileName(file);
}

VpfsFileEngine::~VpfsFileEngine()
{
}

void VpfsFileEngine::setFileName(const QString &file)
{
    Q_D(VpfsFileEngine);
    d->resource.setFileName(file);
}

bool VpfsFileEngine::open(QIODevice::OpenMode flags)
{
    Q_D(VpfsFileEngine);
    if (d->resource.fileName().isEmpty()) {
        qWarning("VpfsFileEngine::open: Missing file name");
        return false;
    }
    if(flags & QIODevice::WriteOnly)
        return false;
    d->uncompress();
    if (!d->resource.isValid()) {
        d->errorString = QSystemError::stdString(ENOENT);
        return false;
    }
    return true;
}

bool VpfsFileEngine::close()
{
    Q_D(VpfsFileEngine);
    d->offset = 0;
    d->uncompressed.clear();
    return true;
}

bool VpfsFileEngine::flush()
{
    return true;
}

qint64 VpfsFileEngine::read(char *data, qint64 len)
{
    Q_D(VpfsFileEngine);
    if(len > size()-d->offset)
        len = size()-d->offset;
    if(len <= 0)
        return 0;
    if(d->resource.isCompressed())
        memcpy(data, d->uncompressed.constData()+d->offset, len);
    else
        memcpy(data, d->resource.data()+d->offset, len);
    d->offset += len;
    return len;
}

qint64 VpfsFileEngine::write(const char *, qint64)
{
    return -1;
}

bool VpfsFileEngine::remove()
{
    return false;
}

bool VpfsFileEngine::copy(const QString &)
{
    return false;
}

bool VpfsFileEngine::rename(const QString &)
{
    return false;
}

bool VpfsFileEngine::link(const QString &)
{
    return false;
}

qint64 VpfsFileEngine::size() const
{
    Q_D(const VpfsFileEngine);
    if(!d->resource.isValid())
        return 0;
    if (d->resource.isCompressed()) {
        d->uncompress();
        return d->uncompressed.size();
    }
    return d->resource.size();
}

qint64 VpfsFileEngine::pos() const
{
    Q_D(const VpfsFileEngine);
    return d->offset;
}

bool VpfsFileEngine::atEnd() const
{
    Q_D(const VpfsFileEngine);
    if(!d->resource.isValid())
        return true;
    return d->offset == size();
}

bool VpfsFileEngine::seek(qint64 pos)
{
    Q_D(VpfsFileEngine);
    if(!d->resource.isValid())
        return false;

    if(d->offset > size())
        return false;
    d->offset = pos;
    return true;
}

bool VpfsFileEngine::isSequential() const
{
    return false;
}

QAbstractFileEngine::FileFlags VpfsFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    Q_D(const VpfsFileEngine);
    QAbstractFileEngine::FileFlags ret = 0;
    if(!d->resource.isValid())
        return ret;

    if(type & PermsMask)
        ret |= QAbstractFileEngine::FileFlags(ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm);
    if(type & TypesMask) {
        if(d->resource.isDir())
            ret |= DirectoryType;
        else
            ret |= FileType;
    }
    if(type & FlagsMask) {
        ret |= ExistsFlag;
        if(d->resource.absoluteFilePath() == QLatin1String(":/"))
            ret |= RootFlag;
    }
    return ret;
}

bool VpfsFileEngine::setPermissions(uint)
{
    return false;
}

QString VpfsFileEngine::fileName(FileName file) const
{
    Q_D(const VpfsFileEngine);
    if(file == BaseName) {
        int slash = d->resource.fileName().lastIndexOf(QLatin1Char('/'));
        if (slash == -1)
            return d->resource.fileName();
        return d->resource.fileName().mid(slash + 1);
    } else if(file == PathName || file == AbsolutePathName) {
        const QString path = (file == AbsolutePathName) ? d->resource.absoluteFilePath() : d->resource.fileName();
        const int slash = path.lastIndexOf(QLatin1Char('/'));
        if (slash == -1)
            return QLatin1String(":");
        else if (slash <= 1)
            return QLatin1String(":/");
        return path.left(slash);

    } else if(file == CanonicalName || file == CanonicalPathName) {
        const QString absoluteFilePath = d->resource.absoluteFilePath();
        if(file == CanonicalPathName) {
            const int slash = absoluteFilePath.lastIndexOf(QLatin1Char('/'));
            if (slash != -1)
                return absoluteFilePath.left(slash);
        }
        return absoluteFilePath;
    }
    return d->resource.fileName();
}

bool VpfsFileEngine::isRelativePath() const
{
    return false;
}

uint VpfsFileEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString VpfsFileEngine::owner(FileOwner) const
{
    return QString();
}

QDateTime VpfsFileEngine::fileTime(FileTime time) const
{
    Q_D(const VpfsFileEngine);
    if (time == ModificationTime)
        return d->resource.lastModified();
    return QDateTime();
}

/*!
    \internal
*/
QAbstractFileEngine::Iterator *VpfsFileEngine::beginEntryList(QDir::Filters filters,
                                                              const QStringList &filterNames)
{
    return new VpfsFileEngineIterator(filters, filterNames);
}

/*!
    \internal
*/
QAbstractFileEngine::Iterator *VpfsFileEngine::endEntryList()
{
    return 0;
}

bool VpfsFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
    Q_D(VpfsFileEngine);
    if (extension == MapExtension) {
        const MapExtensionOption *options = (const MapExtensionOption*)(option);
        MapExtensionReturn *returnValue = static_cast<MapExtensionReturn*>(output);
        returnValue->address = d->map(options->offset, options->size, options->flags);
        return (returnValue->address != 0);
    }
    if (extension == UnMapExtension) {
        const UnMapExtensionOption *options = (const UnMapExtensionOption*)option;
        return d->unmap(options->address);
    }
    return false;
}

bool VpfsFileEngine::supportsExtension(Extension extension) const
{
    return (extension == UnMapExtension || extension == MapExtension);
}

uchar *VpfsFileEnginePrivate::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
    Q_Q(VpfsFileEngine);
    Q_UNUSED(flags);
    if (offset < 0 || size <= 0 || !resource.isValid() || offset + size > resource.size()) {
        q->setError(QFile::UnspecifiedError, QString());
        return 0;
    }
    uchar *address = const_cast<uchar *>(resource.data());
    return (address + offset);
}

bool VpfsFileEnginePrivate::unmap(uchar *ptr)
{
    Q_UNUSED(ptr);
    return true;
}

void VpfsFileEnginePrivate::uncompress() const
{
    if (resource.isCompressed() && uncompressed.isEmpty() && resource.size()) {
#ifndef QT_NO_COMPRESS
        uncompressed = qUncompress(resource.data(), resource.size());
#else
        Q_ASSERT(!"VpfsFileEngine::open: Qt built without support for compression");
#endif
    }
}