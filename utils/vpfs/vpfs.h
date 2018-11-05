#ifndef VPFS_H
#define VPFS_H

#include <QString>
#include <QLocale>
#include <QStringList>
#include <QScopedPointer>

class VpfsPrivate;
class Q_CORE_EXPORT Vpfs
{
public:
    Vpfs(const QString &file=QString(), const QLocale &locale=QLocale());
    ~Vpfs();
    void setFileName(const QString &file);
    QString fileName() const;
    QString absoluteFilePath() const;
    void setLocale(const QLocale &locale);
    QLocale locale() const;
    bool isValid() const;
    bool isCompressed() const;
    qint64 size() const;
    const uchar *data() const;
    QDateTime lastModified() const;
    static void addSearchPath(const QString &path);
    static QStringList searchPaths();
    static bool registerResource(const QString &rccFilename, const QString &resourceRoot=QString());
    static bool unregisterResource(const QString &rccFilename, const QString &resourceRoot=QString());
    static bool registerResource(const uchar *rccData, const QString &resourceRoot=QString());
    static bool unregisterResource(const uchar *rccData, const QString &resourceRoot=QString());
protected:
    friend class VpfsEngine;
    friend class VpfsEngineIterator;
    bool isDir() const;
    inline bool isFile() const { return !isDir(); }
    QStringList children() const;
protected:
    QScopedPointer<VpfsPrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(Vpfs)
};

#endif // VPFS_H