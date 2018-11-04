#ifndef VPFS_H
#define VPFS_H

#include <private/qabstractfileengine_p.h>

#include <QResource>

class Vpfs : public QResource
{
    friend class VpfsFileEngine;
    friend class VpfsFileEngineIterator;

public:
    Vpfs(const QString &file=QString(), const QLocale &locale=QLocale()) : QResource(file, locale)
    {}
};

class VpfsFileEnginePrivate;
class VpfsFileEngine : public QAbstractFileEngine
{
private:
    Q_DECLARE_PRIVATE(VpfsFileEngine)
public:
    explicit VpfsFileEngine(const QString &path);
    ~VpfsFileEngine();

    virtual void setFileName(const QString &file) override;

    virtual bool open(QIODevice::OpenMode flags) override ;
    virtual bool close() override;
    virtual bool flush() override;
    virtual qint64 size() const override;
    virtual qint64 pos() const override;
    virtual bool atEnd() const;
    virtual bool seek(qint64) override;
    virtual qint64 read(char *data, qint64 maxlen) override;
    virtual qint64 write(const char *data, qint64 len) override;

    virtual bool remove() override;
    virtual bool copy(const QString &newName) override;
    virtual bool rename(const QString &newName) override;
    virtual bool link(const QString &newName) override;

    virtual bool isSequential() const override;

    virtual bool isRelativePath() const override;

    virtual bool mkdir(const QString &dirName, bool createParentDirectories) const override;
    virtual bool rmdir(const QString &dirName, bool recurseParentDirectories) const override;

    virtual bool setSize(qint64 size) override;

    virtual QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const override;

    virtual bool caseSensitive() const override;

    virtual FileFlags fileFlags(FileFlags type) const override;

    virtual bool setPermissions(uint perms) override;

    virtual QString fileName(QAbstractFileEngine::FileName file) const override;

    virtual uint ownerId(FileOwner) const override;
    virtual QString owner(FileOwner) const override;

    virtual QDateTime fileTime(FileTime time) const override;

    virtual Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
    virtual Iterator *endEntryList() override;

    bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0) override;
    bool supportsExtension(Extension extension) const override;
};

#endif // VPFS_H
