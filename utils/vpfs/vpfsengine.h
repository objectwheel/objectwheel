#ifndef VPFSENGINE_H
#define VPFSENGINE_H

#include <private/qabstractfileengine_p.h>

class VpfsEngine : public QAbstractFileEngine
{
public:
    explicit VpfsEngine(const QString &path);
    ~VpfsEngine();

    bool open(QIODevice::OpenMode openMode);
    bool close();
    bool flush();
    bool syncToDisk();// ********
    bool seek(qint64 pos);
    bool isSequential() const;
    bool remove();
    bool copy(const QString &newName);
    bool rename(const QString &newName);
    bool renameOverwrite(const QString &newName);// ******
    bool link(const QString &newName);
    bool mkdir(const QString &dirName, bool createParentDirectories) const;
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const;
    bool setSize(qint64 size);
    bool caseSensitive() const;
    bool isRelativePath() const;
    bool setPermissions(uint perms);
    bool setFileTime(const QDateTime &newDate, FileTime time);// *********
    bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0);
    bool supportsExtension(Extension extension) const;
    bool cloneTo(QAbstractFileEngine *target);

    int handle() const;// *********

    void setFileName(const QString &file);// ********

    uint ownerId(FileOwner) const;
    qint64 size() const;
    qint64 pos() const;
    qint64 read(char *data, qint64 maxlen);
    qint64 readLine(char *data, qint64 maxlen);// **********
    qint64 write(const char *data, qint64 len);

    FileFlags fileFlags(FileFlags type=FileInfoAll) const;

    QString owner(FileOwner) const;
    QString fileName(FileName file=DefaultName) const;
    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

    QByteArray id() const;// ********
    QDateTime fileTime(FileTime time) const;

    Iterator *endEntryList();
    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames);

    // bool atEnd() const; // ******** TODO: Make it available via exension function
};

#endif // VPFSENGINE_H
