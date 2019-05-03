#include <usermanager.h>
#include <dirlocker.h>
#include <aes.h>
#include <applicationcore.h>
#include <filesystemutils.h>

#include <QDir>
#include <QCryptographicHash>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QFileInfo>

#define AUTOLOGIN_FILENAME  "alg.lock"
#define AUTOLOGIN_PROTECTOR "QWxsYWggaXMgZ3JlYXRlc3Qu"

namespace {

static QString generateUserDirectory(const QString& user)
{
    return ApplicationCore::userResourcePath() + "/data/" +
            QCryptographicHash::hash(QByteArray().insert(0, user), QCryptographicHash::Md5).toHex();
}

static QString generateToken(const QString& user, const QString& password)
{
    QString hash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha512).toHex();
    auto json = QByteArray().insert(0, QString("{ \"email\" : \"%1\", \"hash\" : \"%2\" }").arg(user).arg(hash));
    return json.toBase64();
}
}

UserManager* UserManager::s_instance = nullptr;
QString UserManager::s_user;
QByteArray UserManager::s_key;

UserManager::UserManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

UserManager::~UserManager()
{
    s_instance = nullptr;
}

UserManager* UserManager::instance()
{
    return s_instance;
}

bool UserManager::exists(const QString& user)
{
    return QFileInfo::exists(generateUserDirectory(user));
}

QString UserManager::dir(const QString& user)
{
    if (!exists(user)) return QString();
    return generateUserDirectory(user);
}

void UserManager::setAutoLogin(const QString& /*password*/)
{
    if (s_user.isEmpty() || dir().isEmpty()) return;
    QString json = "{ \"e\" : \"%1\", \"p\" : \"%2\" }";
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
//  FIXME  wrfile(ApplicationCore::userResourcePath() + "/data/" + AUTOLOGIN_FILENAME,
//        Aes::encrypt(sstep, QByteArray().insert(0, json.arg(s_user, password))));
}

const QString& UserManager::user()
{
    return s_user;
}

const QByteArray& UserManager::key()
{
    return s_key;
}

void UserManager::clearAutoLogin()
{
//  FIXME  QByteArray shredder;
//    for (int i = 1048576; i--;) { shredder.append(QRandomGenerator::global()->generate() % 250); }
//    wrfile(ApplicationCore::userResourcePath() + "/data/" + AUTOLOGIN_FILENAME, shredder);
//    QFile::remove(ApplicationCore::userResourcePath() + "/data/" + AUTOLOGIN_FILENAME);
//    FileSystemUtils::makeFile(ApplicationCore::userResourcePath() + "/data/" + AUTOLOGIN_FILENAME);
}

bool UserManager::hasAutoLogin()
{
//  FIXME  auto algdata = rdfile(ApplicationCore::userResourcePath() + "/data/" + AUTOLOGIN_FILENAME);
    return false /*!algdata.isEmpty()*/;
}

bool UserManager::tryAutoLogin()
{
//  FIXME  if (!hasAutoLogin()) return false;
//    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
//    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
//    auto algdata = rdfile(ApplicationCore::userResourcePath() + "/data/" + AUTOLOGIN_FILENAME);
//    auto jobj = QJsonDocument::fromJson(Aes::decrypt(sstep, algdata)).object();
//    return start(jobj["e"].toString(), jobj["p"].toString());
    return false;
}

bool UserManager::start(const QString& user, const QString& password)
{
    if (s_user == user) {
		return true;
	}

    if (!exists(user)) {
		return false;
	}

    if (!s_user.isEmpty()) {
        stop();
	}

    auto keyHash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha3_512);
    keyHash = QCryptographicHash::hash(keyHash, QCryptographicHash::Md5).toHex();
    s_user = user;
    s_key = keyHash;

//    if (DirLocker::canUnlock(dir(user), keyHash)) {
//        /* Clear all previous trash project folders if locked versions already exists */
//        auto dirlockersFiles = DirLocker::lockFiles();
//        for (const QString& entry : QDir(dir(user)).entryList(QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot)) {
//            /* Check if necessary files */
//            bool breakit = false;
//            for (auto dlentry : dirlockersFiles) {
//                if (entry == dlentry) {
//                    breakit = true;
//                }
//            }

//            if (breakit) continue;

//            const QString& dest = dir(user) + '/' + entry;
//            if (QFileInfo(dest).isDir())
//                return QDir(dest).removeRecursively();
//            else
//                return QFile::remove(dest);
//        }

//        if (!DirLocker::unlock(dir(user), keyHash)) {
//            s_user = "";
//            s_key = "";
//			return false;
//		}
//	}

    emit instance()->started();

    return true;
}

void UserManager::stop()
{
    if (s_user.isEmpty())
		return;

    emit instance()->aboutToStop();

//    if (exists(s_user) && !DirLocker::locked(dir())) {
//        if (!DirLocker::lock(dir(), s_key))
//            qFatal("ProjectManager : Error occurred");
//    }

    s_user = "";
    s_key = "";
}
