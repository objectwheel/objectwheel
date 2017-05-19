#ifndef DIRLOCKER_H
#define DIRLOCKER_H

#include <QObject>

class DirLockerPrivate;

class DirLocker : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(DirLocker)

	public:
		explicit DirLocker(QObject *parent = 0);
        ~DirLocker();
		static DirLocker* instance();
		static bool locked(const QString& dir);
		static bool canUnlock(const QString& dir, const QByteArray& key);
		static bool lock(const QString& dir, const QByteArray& key);
		static bool unlock(const QString& dir, const QByteArray& key);
        static QStringList dirlockersFilenames();

	private:
		static DirLockerPrivate* m_d;
};

#endif // DIRLOCKER_H
