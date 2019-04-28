#include <dirlocker.h>
#include <aes.h>
#include <zipasync.h>

#include <QByteArray>
#include <QFile>
#include <QCryptographicHash>
#include <QDir>

#define CHECK_FILENAME "check.lock"
#define LOCKED_FILENAME "data.lock"
#define ZIPPED_FILENAME "data.tmp"
#define CHECK_SIGN "VEdFZ2FXeGhhR1VnYVd4c1lXeHNZV2dzSUUxMWFHRnRiV1ZrZFc0Z2NtVnpkV3gxYkd4aGFB"

static void clearTrashes(const QString& dir)
{
    QFile::remove(dir + '/' + CHECK_FILENAME);
    QFile::remove(dir + '/' + LOCKED_FILENAME);
    QFile::remove(dir + '/' + ZIPPED_FILENAME);
}

bool DirLocker::locked(const QString& dir)
{
	QString lockedFileName = dir + '/' + LOCKED_FILENAME;
    if (!QFileInfo::exists(lockedFileName)) return false;
	QFile reader(lockedFileName);
	if (!reader.open(QFile::ReadOnly)) qFatal("DirLocker : Error occurred");
	return Aes::encrypted(reader.read(128));
}

bool DirLocker::canUnlock(const QString& /*dir*/, const QByteArray& /*key*/)
{
//   FIXME QString checkFileName = dir + '/' + CHECK_FILENAME;
//    if (!locked(dir) || !QFileInfo::exists(checkFileName)) return false;
//	auto checkData = rdfile(checkFileName);
//	if (!Aes::encrypted(checkData)) return false;
//	auto keyHash = QCryptographicHash::hash(key, QCryptographicHash::Md5).toHex();
//	return (Aes::decrypt(keyHash, checkData) == QString(CHECK_SIGN));
    return false;
}

bool DirLocker::lock(const QString& dir, const QByteArray& key)
{
// FIXME   if (locked(dir)) return false;
//    clearTrashes(dir);
//    QString zippedFileName = dir + '/' + ZIPPED_FILENAME;
//    ZipAsync::zipSync(dir, zippedFileName);
//    auto checkData = QByteArray(CHECK_SIGN);
//    auto zippedData = rdfile(zippedFileName);
//    if (zippedData.size() < 1) return false;
//    auto keyHash = QCryptographicHash::hash(key, QCryptographicHash::Md5).toHex();
//    if (wrfile(dir + '/' + LOCKED_FILENAME, Aes::encrypt(key, zippedData)) < 0) return false;
//    if (wrfile(dir + '/' + CHECK_FILENAME, Aes::encrypt(keyHash, checkData)) < 0) return false;
//    for (auto entry : QDir(dir).entryList(QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot)) {
//        if (entry != LOCKED_FILENAME && entry != CHECK_FILENAME) {
//            const QString& dest = dir + '/' + entry;
//            if (QFileInfo(dest).isDir())
//                return QDir(dest).removeRecursively();
//            else
//                return QFile::remove(dest);
//        }
//    }
    return true;
}

bool DirLocker::unlock(const QString& /*dir*/, const QByteArray& /*key*/)
{
    // FIXME:
    //	if (!canUnlock(dir, key)) return false;
    //	auto zipData = rdfile(dir + '/' + LOCKED_FILENAME);
    //	if (zipData.isEmpty()) return false;
    //	Aes::decrypt(key, zipData);
    //	rm(dir + '/' + LOCKED_FILENAME), rm(dir + '/' + CHECK_FILENAME);
    //	ZipAsync::unzipSync(zipData, dir); // FIXME
    //    return true;
    return false;
}

QStringList DirLocker::lockFiles()
{
    QStringList names;
    names << CHECK_FILENAME;
    names << LOCKED_FILENAME;
    return names;
}
