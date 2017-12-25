#include <userbackend.h>
#include <aes.h>
#include <global.h>
#include <mainwindow.h>
#include <filemanager.h>
#include <dirlocker.h>
#include <projectbackend.h>
#include <limits>
#include <random>

#include <QStandardPaths>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

#define wM (WindowManager::instance())
#define pW (MainWindow::instance()->progressWidget())
#define AUTOLOGIN_FILENAME "alg.inf"
#define AUTOLOGIN_PROTECTOR "QWxsYWggaXMgZ3JlYXRlc3Qu"

static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_int_distribution<int> rand_dist(INT_MIN, INT_MIN);

class UserBackendPrivate
{
	public:
        UserBackendPrivate(UserBackend* uparent);
		QString defaultDataDirectory() const;
		QString generateUserDirectory(const QString& username) const;
        QString generateToken(const QString& username, const QString& password) const;

	public:
        UserBackend* parent = nullptr;
		QString dataDirectory;
        QString currentSessionsToken;
		QString currentSessionsUser;
		QByteArray currentSessionsKey;
		DirLocker dirLocker;
};

UserBackendPrivate::UserBackendPrivate(UserBackend* uparent)
	: parent(uparent)
{
	dataDirectory = defaultDataDirectory();
}

inline QString UserBackendPrivate::defaultDataDirectory() const
{
	QString baseDir;
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
	baseDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).value(0);
#else
	baseDir = QCoreApplication::applicationDirPath();
#endif
	return baseDir + separator() + "data";
}

QString UserBackendPrivate::generateUserDirectory(const QString& username) const
{
    return dataDirectory + separator() + QCryptographicHash::hash(QByteArray().insert(0, username), QCryptographicHash::Md5).toHex();
}

QString UserBackendPrivate::generateToken(const QString& username, const QString& password) const
{
    QString hash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha512).toHex();
    auto json = QByteArray().insert(0, QString("{ \"email\" : \"%1\", \"hash\" : \"%2\" }").arg(username).arg(hash));
    return json.toBase64();
}

UserBackendPrivate* UserBackend::_d = nullptr;

UserBackend::UserBackend(QObject *parent)
	: QObject(parent)
{
	if (_d) return;
    _d = new UserBackendPrivate(this);
}

UserBackend::~UserBackend()
{
    delete _d;
}

UserBackend* UserBackend::instance()
{
	return _d->parent;
}

bool UserBackend::exists(const QString& username)
{
	return ::exists(_d->generateUserDirectory(username));
}

bool UserBackend::buildNewUser(const QString& username)
{
//	if (exists(username)) return false; //True code
//	return mkdir(_d->generateUserDirectory(username));
    //FIXME:
    if (exists(username)) return false; //Bad code
    return mkfile(_d->generateUserDirectory(username) + separator() + "bad.dat");
}

QString UserBackend::dataDirictory()
{
	return _d->dataDirectory;
}

QString UserBackend::userDirectory(const QString& username)
{
	if (!exists(username)) return QString();
    return _d->generateUserDirectory(username);
}

void UserBackend::setAutoLogin(const QString& password)
{
    if (_d->currentSessionsUser.isEmpty() || userDirectory(_d->currentSessionsUser).isEmpty()) return;
    QString json = "{ \"e\" : \"%1\", \"p\" : \"%2\" }";
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    wrfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME,
           Aes::encrypt(sstep, QByteArray().insert(0, json.arg(_d->currentSessionsUser, password))));
}

void UserBackend::clearAutoLogin()
{
    QByteArray shredder;
    for (int i = 1048576; i--;) { shredder.append(rand_dist(mt) % 250); }
    wrfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME, shredder);
    rm(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    mkfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
}

bool UserBackend::hasAutoLogin()
{
    auto algdata = rdfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    return !algdata.isEmpty();
}

bool UserBackend::tryAutoLogin()
{
    if (!hasAutoLogin()) return false;
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    auto algdata = rdfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    auto jobj = QJsonDocument::fromJson(Aes::decrypt(sstep, algdata)).object();
    return startUserSession(jobj["e"].toString(), jobj["p"].toString());
}

QString UserBackend::currentSessionsUser()
{
	return _d->currentSessionsUser;
}

QString UserBackend::currentSessionsToken()
{
    return _d->currentSessionsToken;
}

QString UserBackend::currentSessionsKey()
{
    return _d->currentSessionsKey;
}

QString UserBackend::generateToken(const QString& username, const QString& password)
{
    return _d->generateToken(username, password);
}

bool UserBackend::startUserSession(const QString& username, const QString& password)
{
	if (_d->currentSessionsUser == username) {
		return true;
	}

	if (!exists(username)) {
		return false;
	}

	if (!_d->currentSessionsUser.isEmpty()) {
		stopUserSession();
	}

    auto keyHash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha3_512);
    keyHash = QCryptographicHash::hash(keyHash, QCryptographicHash::Md5).toHex();
	_d->currentSessionsUser = username;
	_d->currentSessionsKey = keyHash;
    _d->currentSessionsToken = _d->generateToken(username, password);

    if (_d->dirLocker.canUnlock(userDirectory(username), keyHash)) {
//        QMetaObject::invokeMethod(pW, "show", Qt::QueuedConnection,
//                                  Q_ARG(QString, "Decryption in progress"));

        /* Clear all previous trash project folders if locked versions already exists */
        auto dirlockersFiles = _d->dirLocker.dirlockersFilenames();
        for (auto entry : ls(userDirectory(username))) {
            /* Check if necessary files */
            bool breakit = false;
            for (auto dlentry : dirlockersFiles) {
                if (entry == dlentry) {
                    breakit = true;
                }
            }

            if (breakit) continue;

            rm(userDirectory(username) + separator() + entry);
        }

		if (!_d->dirLocker.unlock(userDirectory(username), keyHash)) {
			_d->currentSessionsUser = "";
			_d->currentSessionsKey = "";
            _d->currentSessionsToken = "";
			return false;
		}
	}
	return true;
}

void UserBackend::stopUserSession()
{
	if (_d->currentSessionsUser.isEmpty()) {
		return;
	}

    ProjectBackend::instance()->stopProject();

	if (exists(_d->currentSessionsUser) && !_d->dirLocker.locked(userDirectory(_d->currentSessionsUser))) {
		if (!_d->dirLocker.lock(userDirectory(_d->currentSessionsUser), _d->currentSessionsKey)) {
			qFatal("ProjectBackend : Error occurred");
		}
	}

	_d->currentSessionsUser = "";
	_d->currentSessionsKey = "";
    _d->currentSessionsToken = "";
}

