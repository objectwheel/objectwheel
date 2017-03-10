#include <dirlocker.h>
#include <filemanager.h>
#include <aes.h>
#include <zipper.h>
#include <QByteArray>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>

#define CHECK_FILENAME "check.lock"
#define LOCKED_FILENAME "data.lock"
#define CHECK_SIGN "VEdFZ2FXeGhhR1VnYVd4c1lXeHNZV2dzSUUxMWFHRnRiV1ZrZFc0Z2NtVnpkV3gxYkd4aGFB"

class DirLockerPrivate
{
	public:
		DirLockerPrivate(DirLocker* uparent);

	public:
		DirLocker* parent = nullptr;
};

DirLockerPrivate::DirLockerPrivate(DirLocker* uparent)
	: parent(uparent)
{
}

DirLockerPrivate* DirLocker::m_d = nullptr;

DirLocker::DirLocker(QObject *parent)
	: QObject(parent)
{
	if (m_d) return;
	m_d = new DirLockerPrivate(this);
}

DirLocker* DirLocker::instance()
{
	return m_d->parent;
}

bool DirLocker::locked(const QString& dir)
{
	QString lockedFileName = dir + separator() + LOCKED_FILENAME;
	if (!exists(lockedFileName)) return false;
	QFile reader(lockedFileName);
	Q_ASSERT(reader.open(QFile::ReadOnly));
	return Aes::encrypted(reader.read(128));
}

bool DirLocker::canUnlock(const QString& dir, const QByteArray& key)
{
	QString checkFileName = dir + separator() + CHECK_FILENAME;
	if (!locked(dir) || !exists(checkFileName)) return false;
	auto checkData = rdfile(checkFileName);
	if (!Aes::encrypted(checkData)) return false;
	auto keyHash = QCryptographicHash::hash(key, QCryptographicHash::Md5).toHex();
	return (Aes::decrypt(keyHash, checkData) == QString(CHECK_SIGN));
}

bool DirLocker::lock(const QString& dir, const QByteArray& key)
{
	if (locked(dir)) return false;
	QString zippedFileName = dir + separator() + "data.zip";
	Zipper::compressDir(dir, zippedFileName);
	auto checkData = QByteArray(CHECK_SIGN);
	auto zippedData = rdfile(zippedFileName);
	if (zippedData.size() < 1) return false;
	auto keyHash = QCryptographicHash::hash(key, QCryptographicHash::Md5).toHex();
	if (wrfile(dir + separator() + LOCKED_FILENAME, Aes::encrypt(key, zippedData)) < 0) return false;
	if (wrfile(dir + separator() + CHECK_FILENAME, Aes::encrypt(keyHash, checkData)) < 0) return false;
	for (auto entry : ls(dir)) {
		if (entry != LOCKED_FILENAME && entry != CHECK_FILENAME) {
			rm(dir + separator() + entry);
		}
	}
	return true;
}

bool DirLocker::unlock(const QString& dir, const QByteArray& key)
{
	if (!canUnlock(dir, key)) return false;
	auto zipData = rdfile(dir + separator() + LOCKED_FILENAME);
	if (zipData.isEmpty()) return false;
	Aes::decrypt(key, zipData);
	rm(dir + separator() + LOCKED_FILENAME), rm(dir + separator() + CHECK_FILENAME);
	Zipper::extractZip(zipData, dir);
	return true;
}
