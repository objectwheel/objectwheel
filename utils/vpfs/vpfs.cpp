#include <vpfs.h>
#include <vpfs_p.h>

#include <QDir>
#include <QDateTime>
#include <QSet>
#include <QtEndian>

#ifdef Q_OS_UNIX
# include "private/qcore_unix_p.h"
#endif

class QStringSplitter
{
public:
    explicit QStringSplitter(QStringView sv)
        : m_data(sv.data()), m_len(sv.size())
    {
    }
    inline bool hasNext() {
        while (m_pos < m_len && m_data[m_pos] == m_splitChar)
            ++m_pos;
        return m_pos < m_len;
    }
    inline QStringView next() {
        int start = m_pos;
        while (m_pos < m_len && m_data[m_pos] != m_splitChar)
            ++m_pos;
        return QStringView(m_data + start, m_pos - start);
    }
    const QChar *m_data;
    qsizetype m_len;
    qsizetype m_pos = 0;
    QChar m_splitChar = QLatin1Char('/');
};

static QString cleanPath(const QString &_path)
{
    QString path = QDir::cleanPath(_path);
    // QDir::cleanPath does not remove two trailing slashes under _Windows_
    // due to support for UNC paths. Remove those manually.
    if (path.startsWith(QLatin1String("//")))
        path.remove(0, 1);
    return path;
}
Q_DECLARE_TYPEINFO(VpfsRoot, Q_MOVABLE_TYPE);
typedef QList<VpfsRoot*> ResourceList;
struct VpfsGlobalData
{
    QMutex resourceMutex{QMutex::Recursive};
    ResourceList resourceList;
    QStringList resourceSearchPaths;
};
Q_GLOBAL_STATIC(VpfsGlobalData, resourceGlobalData)
static inline QMutex *resourceMutex()
{ return &resourceGlobalData->resourceMutex; }
static inline ResourceList *resourceList()
{ return &resourceGlobalData->resourceList; }
static inline QStringList *resourceSearchPaths()
{ return &resourceGlobalData->resourceSearchPaths; }
/*!
    \class Vpfs
    \inmodule QtCore
    \brief The Vpfs class provides an interface for reading directly from resources.
    \ingroup io
    \reentrant
    \since 4.2
    Vpfs is an object that represents a set of data (and possibly
    children) relating to a single resource entity. Vpfs gives direct
    access to the bytes in their raw format. In this way direct access
    allows reading data without buffer copying or indirection. Indirection
    is often useful when interacting with the resource entity as if it is a
    file, this can be achieved with QFile. The data and children behind a
    Vpfs are normally compiled into an application/library, but it is
    also possible to load a resource at runtime. When loaded at run time
    the resource file will be loaded as one big set of data and then given
    out in pieces via references into the resource tree.
    A Vpfs can either be loaded with an absolute path, either treated
    as a file system rooted with a \c{/} character, or in resource notation
    rooted with a \c{:} character. A relative resource can also be opened
    which will be found in the list of paths returned by QDir::searchPaths().
    A Vpfs that is representing a file will have data backing it, this
    data can possibly be compressed, in which case qUncompress() must be
    used to access the real data; this happens implicitly when accessed
    through a QFile. A Vpfs that is representing a directory will have
    only children and no data.
    \section1 Dynamic Resource Loading
    A resource can be left out of an application's binary and loaded when
    it is needed at run-time by using the registerResource() function. The
    resource file passed into registerResource() must be a binary resource
    as created by rcc. Further information about binary resources can be
    found in \l{The Qt Resource System} documentation.
    This can often be useful when loading a large set of application icons
    that may change based on a setting, or that can be edited by a user and
    later recreated. The resource is immediately loaded into memory, either
    as a result of a single file read operation, or as a memory mapped file.
    This approach can prove to be a significant performance gain as only a
    single file will be loaded, and pieces of data will be given out via the
    path requested in setFileName().
    The unregisterResource() function removes a reference to a particular
    file. If there are Vpfs objects that currently reference resources related
    to the unregistered file, they will continue to be valid but the resource
    file itself will be removed from the resource roots, and thus no further
    Vpfs can be created pointing into this resource data. The resource
    itself will be unmapped from memory when the last Vpfs that points
    to it is destroyed.
    \sa {The Qt Resource System}, QFile, QDir, QFileInfo
*/

void
VpfsPrivate::clear()
{
    absoluteFilePath.clear();
    compressed = 0;
    data = 0;
    size = 0;
    children.clear();
    lastModified = 0;
    container = 0;
    for(int i = 0; i < related.size(); ++i) {
        VpfsRoot *root = related.at(i);
        if(!root->ref.deref())
            delete root;
    }
    related.clear();
}
bool
VpfsPrivate::load(const QString &file)
{
    related.clear();
    QMutexLocker lock(resourceMutex());
    const ResourceList *list = resourceList();
    QString cleaned = cleanPath(file);
    for(int i = 0; i < list->size(); ++i) {
        VpfsRoot *res = list->at(i);
        const int node = res->findNode(cleaned, locale);
        if(node != -1) {
            if(related.isEmpty()) {
                container = res->isContainer(node);
                if(!container) {
                    data = res->data(node, &size);
                    compressed = res->isCompressed(node);
                } else {
                    data = 0;
                    size = 0;
                    compressed = 0;
                }
                lastModified = res->lastModified(node);
            } else if(res->isContainer(node) != container) {
                qWarning("VpfsInfo: Resource [%s] has both data and children!", file.toLatin1().constData());
            }
            res->ref.ref();
            related.append(res);
        } else if(res->mappingRootSubdir(file)) {
            container = true;
            data = 0;
            size = 0;
            compressed = 0;
            lastModified = 0;
            res->ref.ref();
            related.append(res);
        }
    }
    return !related.isEmpty();
}
void
VpfsPrivate::ensureInitialized() const
{
    if(!related.isEmpty())
        return;
    VpfsPrivate *that = const_cast<VpfsPrivate *>(this);
    if(fileName == QLatin1String(":"))
        that->fileName += QLatin1Char('/');
    that->absoluteFilePath = fileName;
    if(!that->absoluteFilePath.startsWith(QLatin1Char(':')))
        that->absoluteFilePath.prepend(QLatin1Char(':'));
    QStringRef path(&fileName);
    if(path.startsWith(QLatin1Char(':')))
        path = path.mid(1);
    if(path.startsWith(QLatin1Char('/'))) {
        that->load(path.toString());
    } else {
        QMutexLocker lock(resourceMutex());
        QStringList searchPaths = *resourceSearchPaths();
        searchPaths << QLatin1String("");
        for(int i = 0; i < searchPaths.size(); ++i) {
            const QString searchPath(searchPaths.at(i) + QLatin1Char('/') + path);
            if(that->load(searchPath)) {
                that->absoluteFilePath = QLatin1Char(':') + searchPath;
                break;
            }
        }
    }
}
void
VpfsPrivate::ensureChildren() const
{
    ensureInitialized();
    if(!children.isEmpty() || !container || related.isEmpty())
        return;
    QString path = absoluteFilePath, k;
    if(path.startsWith(QLatin1Char(':')))
        path = path.mid(1);
    QSet<QString> kids;
    QString cleaned = cleanPath(path);
    for(int i = 0; i < related.size(); ++i) {
        VpfsRoot *res = related.at(i);
        if(res->mappingRootSubdir(path, &k) && !k.isEmpty()) {
            if(!kids.contains(k)) {
                children += k;
                kids.insert(k);
            }
        } else {
            const int node = res->findNode(cleaned);
            if(node != -1) {
                QStringList related_children = res->children(node);
                for(int kid = 0; kid < related_children.size(); ++kid) {
                    k = related_children.at(kid);
                    if(!kids.contains(k)) {
                        children += k;
                        kids.insert(k);
                    }
                }
            }
        }
    }
}
/*!
    Constructs a Vpfs pointing to \a file. \a locale is used to
    load a specific localization of a resource data.
    \sa QFileInfo, QDir::searchPaths(), setFileName(), setLocale()
*/
Vpfs::Vpfs(const QString &file, const QLocale &locale) : d_ptr(new VpfsPrivate(this))
{
    Q_D(Vpfs);
    d->fileName = file;
    d->locale = locale;
}
/*!
    Releases the resources of the Vpfs object.
*/
Vpfs::~Vpfs()
{
}
/*!
    Sets a Vpfs to only load the localization of resource to for \a
    locale. If a resource for the specific locale is not found then the
    C locale is used.
    \sa setFileName()
*/
void Vpfs::setLocale(const QLocale &locale)
{
    Q_D(Vpfs);
    d->clear();
    d->locale = locale;
}
/*!
    Returns the locale used to locate the data for the Vpfs.
*/
QLocale Vpfs::locale() const
{
    Q_D(const Vpfs);
    return d->locale;
}
/*!
    Sets a Vpfs to point to \a file. \a file can either be absolute,
    in which case it is opened directly, if relative then the file will be
    tried to be found in QDir::searchPaths().
    \sa absoluteFilePath()
*/
void Vpfs::setFileName(const QString &file)
{
    Q_D(Vpfs);
    d->clear();
    d->fileName = file;
}
/*!
    Returns the full path to the file that this Vpfs represents as it
    was passed.
    \sa absoluteFilePath()
*/
QString Vpfs::fileName() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return d->fileName;
}
/*!
    Returns the real path that this Vpfs represents, if the resource
    was found via the QDir::searchPaths() it will be indicated in the path.
    \sa fileName()
*/
QString Vpfs::absoluteFilePath() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return d->absoluteFilePath;
}
/*!
    Returns \c true if the resource really exists in the resource hierarchy,
    false otherwise.
*/
bool Vpfs::isValid() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return !d->related.isEmpty();
}
/*!
    \fn bool Vpfs::isFile() const
    Returns \c true if the resource represents a file and thus has data
    backing it, false if it represents a directory.
    \sa isDir()
*/
/*!
    Returns \c true if the resource represents a file and the data backing it
    is in a compressed format, false otherwise.
    \sa data(), isFile()
*/
bool Vpfs::isCompressed() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return d->compressed;
}
/*!
    Returns the size of the data backing the resource.
    \sa data(), isFile()
*/
qint64 Vpfs::size() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return d->size;
}
/*!
    Returns direct access to a read only segment of data that this resource
    represents. If the resource is compressed the data returns is
    compressed and qUncompress() must be used to access the data. If the
    resource is a directory 0 is returned.
    \sa size(), isCompressed(), isFile()
*/
const uchar *Vpfs::data() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return d->data;
}
/*!
    Returns the date and time when the file was last modified before
    packaging into a resource.
*/
QDateTime Vpfs::lastModified() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return d->lastModified ? QDateTime::fromMSecsSinceEpoch(d->lastModified) : QDateTime();
}
/*!
    Returns \c true if the resource represents a directory and thus may have
    children() in it, false if it represents a file.
    \sa isFile()
*/
bool Vpfs::isDir() const
{
    Q_D(const Vpfs);
    d->ensureInitialized();
    return d->container;
}
/*!
    Returns a list of all resources in this directory, if the resource
    represents a file the list will be empty.
    \sa isDir()
*/
QStringList Vpfs::children() const
{
    Q_D(const Vpfs);
    d->ensureChildren();
    return d->children;
}
/*!
  \obsolete
  Use QDir::addSearchPath() with a prefix instead.
  Adds \a path to the search paths searched in to find resources that are
  not specified with an absolute path. The \a path must be an absolute
  path (start with \c{/}).
  The default search path is to search only in the root (\c{:/}). The last
  path added will be consulted first upon next Vpfs creation.
*/
void
Vpfs::addSearchPath(const QString &path)
{
    if (!path.startsWith(QLatin1Char('/'))) {
        qWarning("Vpfs::addResourceSearchPath: Search paths must be absolute (start with /) [%s]",
                 path.toLocal8Bit().data());
        return;
    }
    QMutexLocker lock(resourceMutex());
    resourceSearchPaths()->prepend(path);
}
/*!
  \obsolete
  Use QDir::searchPaths() instead.
  Returns the current search path list. This list is consulted when
  creating a relative resource.
  \sa QDir::addSearchPath(), QDir::setSearchPaths()
*/
QStringList
Vpfs::searchPaths()
{
    QMutexLocker lock(resourceMutex());
    return *resourceSearchPaths();
}
inline uint VpfsRoot::hash(int node) const
{
    if(!node) //root
        return 0;
    const int offset = findOffset(node);
    qint32 name_offset = qFromBigEndian<qint32>(tree + offset);
    name_offset += 2; //jump past name length
    return qFromBigEndian<quint32>(names + name_offset);
}
inline QString VpfsRoot::name(int node) const
{
    if(!node) // root
        return QString();
    const int offset = findOffset(node);
    QString ret;
    qint32 name_offset = qFromBigEndian<qint32>(tree + offset);
    quint16 name_length = qFromBigEndian<qint16>(names + name_offset);
    name_offset += 2;
    name_offset += 4; //jump past hash
    ret.resize(name_length);
    QChar *strData = ret.data();
    qFromBigEndian<ushort>(names + name_offset, name_length, strData);
    return ret;
}
int VpfsRoot::findNode(const QString &_path, const QLocale &locale) const
{
    QString path = _path;
    {
        QString root = mappingRoot();
        if(!root.isEmpty()) {
            if(root == path) {
                path = QLatin1Char('/');
            } else {
                if(!root.endsWith(QLatin1Char('/')))
                    root += QLatin1Char('/');
                if(path.size() >= root.size() && path.startsWith(root))
                    path = path.mid(root.length()-1);
                if(path.isEmpty())
                    path = QLatin1Char('/');
            }
        }
    }
#ifdef DEBUG_RESOURCE_MATCH
    qDebug() << "!!!!" << "START" << path << locale.country() << locale.language();
#endif
    if(path == QLatin1String("/"))
        return 0;
    //the root node is always first
    qint32 child_count = qFromBigEndian<qint32>(tree + 6);
    qint32 child       = qFromBigEndian<qint32>(tree + 10);
    //now iterate up the tree
    int node = -1;
    QStringSplitter splitter(path);
    while (child_count && splitter.hasNext()) {
        QStringView segment = splitter.next();
#ifdef DEBUG_RESOURCE_MATCH
        qDebug() << "  CHILDREN" << segment;
        for(int j = 0; j < child_count; ++j) {
            qDebug() << "   " << child+j << " :: " << name(child+j);
        }
#endif
        const uint h = qt_hash(segment);
        //do the binary search for the hash
        int l = 0, r = child_count-1;
        int sub_node = (l+r+1)/2;
        while(r != l) {
            const uint sub_node_hash = hash(child+sub_node);
            if(h == sub_node_hash)
                break;
            else if(h < sub_node_hash)
                r = sub_node - 1;
            else
                l = sub_node;
            sub_node = (l + r + 1) / 2;
        }
        sub_node += child;
        //now do the "harder" compares
        bool found = false;
        if(hash(sub_node) == h) {
            while(sub_node > child && hash(sub_node-1) == h) //backup for collisions
                --sub_node;
            for(; sub_node < child+child_count && hash(sub_node) == h; ++sub_node) { //here we go...
                if(name(sub_node) == segment) {
                    found = true;
                    int offset = findOffset(sub_node);
#ifdef DEBUG_RESOURCE_MATCH
                    qDebug() << "  TRY" << sub_node << name(sub_node) << offset;
#endif
                    offset += 4;  //jump past name
                    const qint16 flags = qFromBigEndian<qint16>(tree + offset);
                    offset += 2;
                    if(!splitter.hasNext()) {
                        if(!(flags & Directory)) {
                            const qint16 country = qFromBigEndian<qint16>(tree + offset);
                            offset += 2;
                            const qint16 language = qFromBigEndian<qint16>(tree + offset);
                            offset += 2;
#ifdef DEBUG_RESOURCE_MATCH
                            qDebug() << "    " << "LOCALE" << country << language;
#endif
                            if(country == locale.country() && language == locale.language()) {
#ifdef DEBUG_RESOURCE_MATCH
                                qDebug() << "!!!!" << "FINISHED" << __LINE__ << sub_node;
#endif
                                return sub_node;
                            } else if((country == QLocale::AnyCountry && language == locale.language()) ||
                                      (country == QLocale::AnyCountry && language == QLocale::C && node == -1)) {
                                node = sub_node;
                            }
                            continue;
                        } else {
#ifdef DEBUG_RESOURCE_MATCH
                            qDebug() << "!!!!" << "FINISHED" << __LINE__ << sub_node;
#endif
                            return sub_node;
                        }
                    }
                    if(!(flags & Directory))
                        return -1;
                    child_count = qFromBigEndian<qint32>(tree + offset);
                    offset += 4;
                    child = qFromBigEndian<qint32>(tree + offset);
                    break;
                }
            }
        }
        if(!found)
            break;
    }
#ifdef DEBUG_RESOURCE_MATCH
    qDebug() << "!!!!" << "FINISHED" << __LINE__ << node;
#endif
    return node;
}
short VpfsRoot::flags(int node) const
{
    if(node == -1)
        return 0;
    const int offset = findOffset(node) + 4; //jump past name
    return qFromBigEndian<qint16>(tree + offset);
}
const uchar *VpfsRoot::data(int node, qint64 *size) const
{
    if(node == -1) {
        *size = 0;
        return 0;
    }
    int offset = findOffset(node) + 4; //jump past name
    const qint16 flags = qFromBigEndian<qint16>(tree + offset);
    offset += 2;
    offset += 4; //jump past locale
    if(!(flags & Directory)) {
        const qint32 data_offset = qFromBigEndian<qint32>(tree + offset);
        const quint32 data_length = qFromBigEndian<quint32>(payloads + data_offset);
        const uchar *ret = payloads+data_offset+4;
        *size = data_length;
        return ret;
    }
    *size = 0;
    return 0;
}
quint64 VpfsRoot::lastModified(int node) const
{
    if (node == -1 || version < 0x02)
        return 0;
    const int offset = findOffset(node) + 14;
    return qFromBigEndian<quint64>(tree + offset);
}
QStringList VpfsRoot::children(int node) const
{
    if(node == -1)
        return QStringList();
    int offset = findOffset(node) + 4; //jump past name
    const qint16 flags = qFromBigEndian<qint16>(tree + offset);
    offset += 2;
    QStringList ret;
    if(flags & Directory) {
        const qint32 child_count = qFromBigEndian<qint32>(tree + offset);
        offset += 4;
        const qint32 child_off = qFromBigEndian<qint32>(tree + offset);
        ret.reserve(child_count);
        for(int i = child_off; i < child_off+child_count; ++i)
            ret << name(i);
    }
    return ret;
}
bool VpfsRoot::mappingRootSubdir(const QString &path, QString *match) const
{
    const QString root = mappingRoot();
    if (root.isEmpty())
        return false;
    QStringSplitter rootIt(root);
    QStringSplitter pathIt(path);
    while (rootIt.hasNext()) {
        if (pathIt.hasNext()) {
            if (rootIt.next() != pathIt.next()) // mismatch
                return false;
        } else {
            // end of path, but not of root:
            if (match)
                *match = rootIt.next().toString();
            return true;
        }
    }
    // end of root
    return !pathIt.hasNext();
}
Q_CORE_EXPORT bool qRegisterResourceData(int version, const unsigned char *tree,
                                         const unsigned char *name, const unsigned char *data)
{
    QMutexLocker lock(resourceMutex());
    if ((version == 0x01 || version == 0x2) && resourceList()) {
        bool found = false;
        VpfsRoot res(version, tree, name, data);
        for(int i = 0; i < resourceList()->size(); ++i) {
            if(*resourceList()->at(i) == res) {
                found = true;
                break;
            }
        }
        if(!found) {
            VpfsRoot *root = new VpfsRoot(version, tree, name, data);
            root->ref.ref();
            resourceList()->append(root);
        }
        return true;
    }
    return false;
}
Q_CORE_EXPORT bool qUnregisterResourceData(int version, const unsigned char *tree,
                                           const unsigned char *name, const unsigned char *data)
{
    if (resourceGlobalData.isDestroyed())
        return false;
    QMutexLocker lock(resourceMutex());
    if ((version == 0x01 || version == 0x02) && resourceList()) {
        VpfsRoot res(version, tree, name, data);
        for(int i = 0; i < resourceList()->size(); ) {
            if(*resourceList()->at(i) == res) {
                VpfsRoot *root = resourceList()->takeAt(i);
                if(!root->ref.deref())
                    delete root;
            } else {
                ++i;
            }
        }
        return true;
    }
    return false;
}
//run time resource creation
class QDynamicBufferResourceRoot: public VpfsRoot
{
    QString root;
    const uchar *buffer;
public:
    inline QDynamicBufferResourceRoot(const QString &_root) : root(_root), buffer(0) { }
    inline ~QDynamicBufferResourceRoot() { }
    inline const uchar *mappingBuffer() const { return buffer; }
    QString mappingRoot() const override { return root; }
    ResourceRootType type() const override { return Resource_Buffer; }
    // size == -1 means "unknown"
    bool registerSelf(const uchar *b, int size)
    {
        // 5 int "pointers"
        if (size >= 0 && size < 20)
            return false;
        //setup the data now
        int offset = 0;
        //magic number
        if(b[offset+0] != 'q' || b[offset+1] != 'r' ||
           b[offset+2] != 'e' || b[offset+3] != 's') {
            return false;
        }
        offset += 4;
        const int version = qFromBigEndian<qint32>(b + offset);
        offset += 4;
        const int tree_offset = qFromBigEndian<qint32>(b + offset);
        offset += 4;
        const int data_offset = qFromBigEndian<qint32>(b + offset);
        offset += 4;
        const int name_offset = qFromBigEndian<qint32>(b + offset);
        offset += 4;
        // Some sanity checking for sizes. This is _not_ a security measure.
        if (size >= 0 && (tree_offset >= size || data_offset >= size || name_offset >= size))
            return false;
        if (version == 0x01 || version == 0x02) {
            buffer = b;
            setSource(version, b+tree_offset, b+name_offset, b+data_offset);
            return true;
        }
        return false;
    }
};
#if defined(Q_OS_UNIX) && !defined (Q_OS_NACL) && !defined(Q_OS_INTEGRITY)
#define QT_USE_MMAP
#endif
// most of the headers below are already included in qplatformdefs.h
// also this lacks Large File support but that's probably irrelevant
#if defined(QT_USE_MMAP)
// for mmap
QT_BEGIN_INCLUDE_NAMESPACE
#include <sys/mman.h>
#include <errno.h>
QT_END_INCLUDE_NAMESPACE
#endif
class QDynamicFileResourceRoot: public QDynamicBufferResourceRoot
{
    QString fileName;
    // for mmap'ed files, this is what needs to be unmapped.
    uchar *unmapPointer;
    unsigned int unmapLength;
public:
    inline QDynamicFileResourceRoot(const QString &_root) : QDynamicBufferResourceRoot(_root), unmapPointer(0), unmapLength(0) { }
    ~QDynamicFileResourceRoot() {
#if defined(QT_USE_MMAP)
        if (unmapPointer) {
            munmap((char*)unmapPointer, unmapLength);
            unmapPointer = 0;
            unmapLength = 0;
        } else
#endif
        {
            delete [] mappingBuffer();
        }
    }
    QString mappingFile() const { return fileName; }
    ResourceRootType type() const override { return Resource_File; }
    bool registerSelf(const QString &f) {
        bool fromMM = false;
        uchar *data = 0;
        unsigned int data_len = 0;
#ifdef QT_USE_MMAP
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif
        int fd = QT_OPEN(QFile::encodeName(f), O_RDONLY,
#if defined(Q_OS_WIN)
                         _S_IREAD | _S_IWRITE
#else
                         0666
#endif
            );
        if (fd >= 0) {
            QT_STATBUF st;
            if (!QT_FSTAT(fd, &st)) {
                uchar *ptr;
                ptr = reinterpret_cast<uchar *>(
                    mmap(0, st.st_size,             // any address, whole file
                         PROT_READ,                 // read-only memory
                         MAP_FILE | MAP_PRIVATE,    // swap-backed map from file
                         fd, 0));                   // from offset 0 of fd
                if (ptr && ptr != reinterpret_cast<uchar *>(MAP_FAILED)) {
                    data = ptr;
                    data_len = st.st_size;
                    fromMM = true;
                }
            }
            ::close(fd);
        }
#endif // QT_USE_MMAP
        if(!data) {
            QFile file(f);
            if (!file.exists())
                return false;
            data_len = file.size();
            data = new uchar[data_len];
            bool ok = false;
            if (file.open(QIODevice::ReadOnly))
                ok = (data_len == (uint)file.read((char*)data, data_len));
            if (!ok) {
                delete [] data;
                data = 0;
                data_len = 0;
                return false;
            }
            fromMM = false;
        }
        if (data && QDynamicBufferResourceRoot::registerSelf(data, data_len)) {
            if(fromMM) {
                unmapPointer = data;
                unmapLength = data_len;
            }
            fileName = f;
            return true;
        }
        return false;
    }
};
static QString qt_resource_fixResourceRoot(QString r) {
    if(!r.isEmpty()) {
        if(r.startsWith(QLatin1Char(':')))
            r = r.mid(1);
        if(!r.isEmpty())
            r = QDir::cleanPath(r);
    }
    return r;
}
/*!
   \fn bool Vpfs::registerResource(const QString &rccFileName, const QString &mapRoot)
   Registers the resource with the given \a rccFileName at the location in the
   resource tree specified by \a mapRoot, and returns \c true if the file is
   successfully opened; otherwise returns \c false.
   \sa unregisterResource()
*/
bool
Vpfs::registerResource(const QString &rccFilename, const QString &resourceRoot)
{
    QString r = qt_resource_fixResourceRoot(resourceRoot);
    if(!r.isEmpty() && r[0] != QLatin1Char('/')) {
        qWarning("QDir::registerResource: Registering a resource [%s] must be rooted in an absolute path (start with /) [%s]",
                 rccFilename.toLocal8Bit().data(), resourceRoot.toLocal8Bit().data());
        return false;
    }
    QDynamicFileResourceRoot *root = new QDynamicFileResourceRoot(r);
    if(root->registerSelf(rccFilename)) {
        root->ref.ref();
        QMutexLocker lock(resourceMutex());
        resourceList()->append(root);
        return true;
    }
    delete root;
    return false;
}
/*!
  \fn bool Vpfs::unregisterResource(const QString &rccFileName, const QString &mapRoot)
  Unregisters the resource with the given \a rccFileName at the location in
  the resource tree specified by \a mapRoot, and returns \c true if the
  resource is successfully unloaded and no references exist for the
  resource; otherwise returns \c false.
  \sa registerResource()
*/
bool
Vpfs::unregisterResource(const QString &rccFilename, const QString &resourceRoot)
{
    QString r = qt_resource_fixResourceRoot(resourceRoot);
    QMutexLocker lock(resourceMutex());
    ResourceList *list = resourceList();
    for(int i = 0; i < list->size(); ++i) {
        VpfsRoot *res = list->at(i);
        if(res->type() == VpfsRoot::Resource_File) {
            QDynamicFileResourceRoot *root = reinterpret_cast<QDynamicFileResourceRoot*>(res);
            if (root->mappingFile() == rccFilename && root->mappingRoot() == r) {
                resourceList()->removeAt(i);
                if(!root->ref.deref()) {
                    delete root;
                    return true;
                }
                return false;
            }
        }
    }
    return false;
}
/*!
   \fn bool Vpfs::registerResource(const uchar *rccData, const QString &mapRoot)
   \since 4.3
   Registers the resource with the given \a rccData at the location in the
   resource tree specified by \a mapRoot, and returns \c true if the file is
   successfully opened; otherwise returns \c false.
   \warning The data must remain valid throughout the life of any QFile
   that may reference the resource data.
   \sa unregisterResource()
*/
bool
Vpfs::registerResource(const uchar *rccData, const QString &resourceRoot)
{
    QString r = qt_resource_fixResourceRoot(resourceRoot);
    if(!r.isEmpty() && r[0] != QLatin1Char('/')) {
        qWarning("QDir::registerResource: Registering a resource [%p] must be rooted in an absolute path (start with /) [%s]",
                 rccData, resourceRoot.toLocal8Bit().data());
        return false;
    }
    QDynamicBufferResourceRoot *root = new QDynamicBufferResourceRoot(r);
    if (root->registerSelf(rccData, -1)) {
        root->ref.ref();
        QMutexLocker lock(resourceMutex());
        resourceList()->append(root);
        return true;
    }
    delete root;
    return false;
}
/*!
  \fn bool Vpfs::unregisterResource(const uchar *rccData, const QString &mapRoot)
  \since 4.3
  Unregisters the resource with the given \a rccData at the location in the
  resource tree specified by \a mapRoot, and returns \c true if the resource is
  successfully unloaded and no references exist into the resource; otherwise returns \c false.
  \sa registerResource()
*/
bool
Vpfs::unregisterResource(const uchar *rccData, const QString &resourceRoot)
{
    QString r = qt_resource_fixResourceRoot(resourceRoot);
    QMutexLocker lock(resourceMutex());
    ResourceList *list = resourceList();
    for(int i = 0; i < list->size(); ++i) {
        VpfsRoot *res = list->at(i);
        if(res->type() == VpfsRoot::Resource_Buffer) {
            QDynamicBufferResourceRoot *root = reinterpret_cast<QDynamicBufferResourceRoot*>(res);
            if (root->mappingBuffer() == rccData && root->mappingRoot() == r) {
                resourceList()->removeAt(i);
                if(!root->ref.deref()) {
                    delete root;
                    return true;
                }
                return false;
            }
        }
    }
    return false;
}