#ifndef VPFS_P_H
#define VPFS_P_H

class VpfsRoot
{
    enum Flags
    {
        Compressed = 0x01,
        Directory = 0x02
    };
    const uchar *tree, *names, *payloads;
    int version;
    inline int findOffset(int node) const { return node * (14 + (version >= 0x02 ? 8 : 0)); } //sizeof each tree element
    uint hash(int node) const;
    QString name(int node) const;
    short flags(int node) const;
public:
    mutable QAtomicInt ref;
    inline VpfsRoot(): tree(0), names(0), payloads(0), version(0) {}
    inline VpfsRoot(int version, const uchar *t, const uchar *n, const uchar *d) { setSource(version, t, n, d); }
    virtual ~VpfsRoot() { }
    int findNode(const QString &path, const QLocale &locale=QLocale()) const;
    inline bool isContainer(int node) const { return flags(node) & Directory; }
    inline bool isCompressed(int node) const { return flags(node) & Compressed; }
    const uchar *data(int node, qint64 *size) const;
    quint64 lastModified(int node) const;
    QStringList children(int node) const;
    virtual QString mappingRoot() const { return QString(); }
    bool mappingRootSubdir(const QString &path, QString *match=0) const;
    inline bool operator==(const VpfsRoot &other) const
    { return tree == other.tree && names == other.names && payloads == other.payloads && version == other.version; }
    inline bool operator!=(const VpfsRoot &other) const
    { return !operator==(other); }
    enum ResourceRootType { Resource_Builtin, Resource_File, Resource_Buffer };
    virtual ResourceRootType type() const { return Resource_Builtin; }
protected:
    inline void setSource(int v, const uchar *t, const uchar *n, const uchar *d) {
        tree = t;
        names = n;
        payloads = d;
        version = v;
    }
};

class VpfsPrivate {
public:
    inline VpfsPrivate(Vpfs *_q) : q_ptr(_q) { clear(); }
    inline ~VpfsPrivate() { clear(); }
    void ensureInitialized() const;
    void ensureChildren() const;
    bool load(const QString &file);
    void clear();
    QLocale locale;
    QString fileName, absoluteFilePath;
    QList<VpfsRoot*> related;
    uint container : 1;
    mutable uint compressed : 1;
    mutable qint64 size;
    mutable const uchar *data;
    mutable QStringList children;
    mutable quint64 lastModified;
    Vpfs *q_ptr;
    Q_DECLARE_PUBLIC(Vpfs)
};

#endif // VPFS_P_H
