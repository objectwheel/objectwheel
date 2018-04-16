#ifndef DIRLOCKER_H
#define DIRLOCKER_H

#include <QString>

class DirLocker
{
    Q_DISABLE_COPY(DirLocker)

public:
    static QStringList lockFiles();
    static bool locked(const QString& dir);
    static bool lock(const QString& dir, const QByteArray& key);
    static bool unlock(const QString& dir, const QByteArray& key);
    static bool canUnlock(const QString& dir, const QByteArray& key);

private:
    DirLocker() {}
};

#endif // DIRLOCKER_H
