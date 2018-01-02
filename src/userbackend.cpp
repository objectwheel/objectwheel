#include <userbackend.h>
#include <filemanager.h>
#include <dirlocker.h>
#include <limits>
#include <random>
#include <aes.h>

#include <QStandardPaths>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

#define AUTOLOGIN_FILENAME  "alg.lock"
#define AUTOLOGIN_PROTECTOR "QWxsYWggaXMgZ3JlYXRlc3Qu"

static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_int_distribution<int> rand_dist(INT_MIN, INT_MIN);

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

UserBackend* UserBackend::instance()
{
    static UserBackend instance;
    return &instance;
}

bool UserBackend::exists(const QString& user)
{
    return ::exists(generateUserDirectory(user));
}

bool UserBackend::newUser(const QString& user)
{
//	if (exists(user)) return false; //True code
//	return mkdir(generateUserDirectory(user));
    //FIXME:
    if (exists(user)) return false; //Bad code
    return mkfile(generateUserDirectory(user) + separator() + "bad.dat");
}

QString UserBackend::dir(const QString& user)
{
    if (!exists(user)) return QString();
    return generateUserDirectory(user);
}

void UserBackend::setAutoLogin(const QString& password)
{
    if (_user.isEmpty() || dir().isEmpty()) return;
    QString json = "{ \"e\" : \"%1\", \"p\" : \"%2\" }";
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    wrfile(ddd() + separator() + AUTOLOGIN_FILENAME,
        Aes::encrypt(sstep, QByteArray().insert(0, json.arg(_user, password))));
}

const QString& UserBackend::user() const
{
    return _user;
}

const QString& UserBackend::token() const
{
    return _token;
}

const QByteArray& UserBackend::key() const
{
    return _key;
}

void UserBackend::clearAutoLogin()
{
    QByteArray shredder;
    for (int i = 1048576; i--;) { shredder.append(rand_dist(mt) % 250); }
    wrfile(ddd() + separator() + AUTOLOGIN_FILENAME, shredder);
    rm(ddd() + separator() + AUTOLOGIN_FILENAME);
    mkfile(ddd() + separator() + AUTOLOGIN_FILENAME);
}

bool UserBackend::hasAutoLogin()
{
    auto algdata = rdfile(ddd() + separator() + AUTOLOGIN_FILENAME);
    return !algdata.isEmpty();
}

bool UserBackend::tryAutoLogin()
{
    if (!hasAutoLogin()) return false;
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    auto algdata = rdfile(ddd() + separator() + AUTOLOGIN_FILENAME);
    auto jobj = QJsonDocument::fromJson(Aes::decrypt(sstep, algdata)).object();
    return start(jobj["e"].toString(), jobj["p"].toString());
}

bool UserBackend::start(const QString& user, const QString& password)
{
    if (_user == user) {
		return true;
	}

    if (!exists(user)) {
		return false;
	}

    if (!_user.isEmpty()) {
        stop();
	}

    auto keyHash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha3_512);
    keyHash = QCryptographicHash::hash(keyHash, QCryptographicHash::Md5).toHex();
    _user = user;
    _key = keyHash;
    _token = generateToken(user, password);

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
            _user = "";
            _key = "";
            _token = "";
			return false;
		}
	}
	return true;
}

void UserBackend::stop()
{
    if (_user.isEmpty())
		return;

    emit aboutToStop();

    if (exists(_user) && !DirLocker::locked(dir()))
        if (!DirLocker::lock(dir(), _key))
            qFatal("ProjectBackend : Error occurred");

    _user = "";
    _key = "";
    _token = "";
}
