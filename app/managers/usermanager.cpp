#include <usermanager.h>
#include <filemanager.h>
#include <dirlocker.h>
#include <aes.h>

#include <QStandardPaths>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#define AUTOLOGIN_FILENAME  "alg.lock"
#define AUTOLOGIN_PROTECTOR "QWxsYWggaXMgZ3JlYXRlc3Qu"

namespace {

// Default data directory
static inline QString ddd()
{
	QString baseDir;
    #if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE) || defined(Q_OS_WIN)
	baseDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).value(0);
    #else
	baseDir = QCoreApplication::applicationDirPath();
    #endif
	return baseDir + separator() + "data";
}

static QString generateUserDirectory(const QString& user)
{
    return ddd() + separator() + QCryptographicHash::hash(QByteArray().insert(0, user), QCryptographicHash::Md5).toHex();
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
QString UserManager::s_token;
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
    return ::exists(generateUserDirectory(user));
}

bool UserManager::newUser(const QString& user)
{
//	if (exists(user)) return false; //True code
//	return mkdir(generateUserDirectory(user));
    //FIXME:
    if (exists(user)) return false; //Bad code
    return mkfile(generateUserDirectory(user) + separator() + "bad.dat");
}

QString UserManager::dir(const QString& user)
{
    if (!exists(user)) return QString();
    return generateUserDirectory(user);
}

void UserManager::setAutoLogin(const QString& password)
{
    if (s_user.isEmpty() || dir().isEmpty()) return;
    QString json = "{ \"e\" : \"%1\", \"p\" : \"%2\" }";
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    wrfile(ddd() + separator() + AUTOLOGIN_FILENAME,
        Aes::encrypt(sstep, QByteArray().insert(0, json.arg(s_user, password))));
}

const QString& UserManager::user()
{
    return s_user;
}

const QString& UserManager::token()
{
    return s_token;
}

const QByteArray& UserManager::key()
{
    return s_key;
}

void UserManager::clearAutoLogin()
{
    QByteArray shredder;
    for (int i = 1048576; i--;) { shredder.append(QRandomGenerator::global()->generate() % 250); }
    wrfile(ddd() + separator() + AUTOLOGIN_FILENAME, shredder);
    rm(ddd() + separator() + AUTOLOGIN_FILENAME);
    mkfile(ddd() + separator() + AUTOLOGIN_FILENAME);
}

bool UserManager::hasAutoLogin()
{
    auto algdata = rdfile(ddd() + separator() + AUTOLOGIN_FILENAME);
    return !algdata.isEmpty();
}

bool UserManager::tryAutoLogin()
{
    if (!hasAutoLogin()) return false;
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    auto algdata = rdfile(ddd() + separator() + AUTOLOGIN_FILENAME);
    auto jobj = QJsonDocument::fromJson(Aes::decrypt(sstep, algdata)).object();
    return start(jobj["e"].toString(), jobj["p"].toString());
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
    s_token = generateToken(user, password);

    if (DirLocker::canUnlock(dir(user), keyHash)) {
        /* Clear all previous trash project folders if locked versions already exists */
        auto dirlockersFiles = DirLocker::lockFiles();
        for (auto entry : ls(dir(user))) {
            /* Check if necessary files */
            bool breakit = false;
            for (auto dlentry : dirlockersFiles) {
                if (entry == dlentry) {
                    breakit = true;
                }
            }

            if (breakit) continue;

            rm(dir(user) + separator() + entry);
        }

        if (!DirLocker::unlock(dir(user), keyHash)) {
            s_user = "";
            s_key = "";
            s_token = "";
			return false;
		}
	}

    emit instance()->started();

    return true;
}

void UserManager::stop()
{
    if (s_user.isEmpty())
		return;

    emit instance()->aboutToStop();

    if (exists(s_user) && !DirLocker::locked(dir())) {
        if (!DirLocker::lock(dir(), s_key))
            qFatal("ProjectManager : Error occurred");
    }

    s_user = "";
    s_key = "";
    s_token = "";
}
