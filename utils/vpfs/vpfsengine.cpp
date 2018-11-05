#include <vpfsengine.h>
#include <vpfsengine_p.h>
#include <vpfsengineiterator.h>

#include <private/qsystemerror_p.h>

#ifdef Q_OS_UNIX
# include <private/qcore_unix_p.h>
#endif

#include <QDateTime>

bool VpfsEngine::mkdir(const QString &, bool) const
{
    return false;
}

bool VpfsEngine::rmdir(const QString &, bool) const
{
    return false;
}

bool VpfsEngine::setSize(qint64)
{
    return false;
}

QStringList VpfsEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    return QAbstractFileEngine::entryList(filters, filterNames);
}

bool VpfsEngine::caseSensitive() const
{
    return true;
}

VpfsEngine::VpfsEngine(const QString &file) : QAbstractFileEngine(*new VpfsEnginePrivate)
{
    Q_D(VpfsEngine);
    d->resource.setFileName(file);
}

VpfsEngine::~VpfsEngine()
{
}

void VpfsEngine::setFileName(const QString &file)
{
    Q_D(VpfsEngine);
    d->resource.setFileName(file);
}

bool VpfsEngine::open(QIODevice::OpenMode flags)
{
    Q_D(VpfsEngine);
    if (d->resource.fileName().isEmpty()) {
        qWarning("VpfsEngine::open: Missing file name");
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

bool VpfsEngine::close()
{
    Q_D(VpfsEngine);
    d->offset = 0;
    d->uncompressed.clear();
    return true;
}

bool VpfsEngine::flush()
{
    return true;
}

qint64 VpfsEngine::read(char* data, qint64 len)
{
    Q_D(VpfsEngine);
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

qint64 VpfsEngine::write(const char* , qint64)
{
    return -1;
}

bool VpfsEngine::remove()
{
    return false;
}

bool VpfsEngine::copy(const QString &)
{
    return false;
}

bool VpfsEngine::rename(const QString &)
{
    return false;
}

bool VpfsEngine::link(const QString &)
{
    return false;
}

qint64 VpfsEngine::size() const
{
    Q_D(const VpfsEngine);
    if(!d->resource.isValid())
        return 0;
    if (d->resource.isCompressed()) {
        d->uncompress();
        return d->uncompressed.size();
    }
    return d->resource.size();
}

qint64 VpfsEngine::pos() const
{
    Q_D(const VpfsEngine);
    return d->offset;
}

bool VpfsEngine::atEnd() const
{
    Q_D(const VpfsEngine);
    if(!d->resource.isValid())
        return true;
    return d->offset == size();
}

bool VpfsEngine::seek(qint64 pos)
{
    Q_D(VpfsEngine);
    if(!d->resource.isValid())
        return false;

    if(d->offset > size())
        return false;
    d->offset = pos;
    return true;
}

bool VpfsEngine::isSequential() const
{
    return false;
}

QAbstractFileEngine::FileFlags VpfsEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    Q_D(const VpfsEngine);
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

bool VpfsEngine::setPermissions(uint)
{
    return false;
}

QString VpfsEngine::fileName(FileName file) const
{
    Q_D(const VpfsEngine);
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

bool VpfsEngine::isRelativePath() const
{
    return false;
}

uint VpfsEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString VpfsEngine::owner(FileOwner) const
{
    return QString();
}

QDateTime VpfsEngine::fileTime(FileTime time) const
{
    Q_D(const VpfsEngine);
    if (time == ModificationTime)
        return d->resource.lastModified();
    return QDateTime();
}

QAbstractFileEngine::Iterator* VpfsEngine::beginEntryList(QDir::Filters filters,
                                                              const QStringList &filterNames)
{
    return new VpfsEngineIterator(filters, filterNames);
}

QAbstractFileEngine::Iterator* VpfsEngine::endEntryList()
{
    return 0;
}

bool VpfsEngine::extension(Extension extension, const ExtensionOption* option, ExtensionReturn* output)
{
    Q_D(VpfsEngine);
    if (extension == MapExtension) {
        const MapExtensionOption* options = (const MapExtensionOption*)(option);
        MapExtensionReturn* returnValue = static_cast<MapExtensionReturn*>(output);
        returnValue->address = d->map(options->offset, options->size, options->flags);
        return (returnValue->address != 0);
    }
    if (extension == UnMapExtension) {
        const UnMapExtensionOption* options = (const UnMapExtensionOption*)option;
        return d->unmap(options->address);
    }
    return false;
}

bool VpfsEngine::supportsExtension(Extension extension) const
{
    return (extension == UnMapExtension || extension == MapExtension);
}

uchar* VpfsEnginePrivate::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
    Q_Q(VpfsEngine);
    Q_UNUSED(flags);
    if (offset < 0 || size <= 0 || !resource.isValid() || offset + size > resource.size()) {
        q->setError(QFile::UnspecifiedError, QString());
        return 0;
    }
    uchar* address = const_cast<uchar*>(resource.data());
    return (address + offset);
}

bool VpfsEnginePrivate::unmap(uchar* ptr)
{
    Q_UNUSED(ptr);
    return true;
}

void VpfsEnginePrivate::uncompress() const
{
    if (resource.isCompressed() && uncompressed.isEmpty() && resource.size())
        uncompressed = qUncompress(resource.data(), resource.size());
}