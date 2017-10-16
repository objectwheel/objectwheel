#include <usermanager.h>
#include <QStandardPaths>
#include <QCoreApplication>
#include <filemanager.h>
#include <QCryptographicHash>
#include <QByteArray>
#include <dirlocker.h>
#include <projectmanager.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <aes.h>
#include <global.h>
#include <mainwindow.h>

#define cW (MainWindow::instance()->centralWidget())
#define pW (MainWindow::instance()->progressWidget())
#define AUTOLOGIN_FILENAME "alg.inf"
#define AUTOLOGIN_PROTECTOR "QWxsYWggaXMgZ3JlYXRlc3Qu"

class UserManagerPrivate
{
	public:
		UserManagerPrivate(UserManager* uparent);
		QString defaultDataDirectory() const;
		QString generateUserDirectory(const QString& username) const;
        QString generateToken(const QString& username, const QString& password) const;

	public:
		UserManager* parent = nullptr;
		QString dataDirectory;
        QString currentSessionsToken;
		QString currentSessionsUser;
		QByteArray currentSessionsKey;
		DirLocker dirLocker;
};

UserManagerPrivate::UserManagerPrivate(UserManager* uparent)
	: parent(uparent)
{
	dataDirectory = defaultDataDirectory();
}

inline QString UserManagerPrivate::defaultDataDirectory() const
{
	QString baseDir;
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
	baseDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).value(0);
#else
	baseDir = QCoreApplication::applicationDirPath();
#endif
	return baseDir + separator() + "data";
}

QString UserManagerPrivate::generateUserDirectory(const QString& username) const
{
    return dataDirectory + separator() + QCryptographicHash::hash(QByteArray().insert(0, username), QCryptographicHash::Md5).toHex();
}

QString UserManagerPrivate::generateToken(const QString& username, const QString& password) const
{
    QString hash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha512).toHex();
    auto json = QByteArray().insert(0, QString("{ \"email\" : \"%1\", \"hash\" : \"%2\" }").arg(username).arg(hash));
    return json.toBase64();
}

UserManagerPrivate* UserManager::_d = nullptr;

UserManager::UserManager(QObject *parent)
	: QObject(parent)
{
	if (_d) return;
    _d = new UserManagerPrivate(this);
}

UserManager::~UserManager()
{
    delete _d;
}

UserManager* UserManager::instance()
{
	return _d->parent;
}

bool UserManager::exists(const QString& username)
{
	return ::exists(_d->generateUserDirectory(username));
}

bool UserManager::buildNewUser(const QString& username)
{
//	if (exists(username)) return false; //True code
//	return mkdir(_d->generateUserDirectory(username));
    //FIXME:
    if (exists(username)) return false; //Bad code
    return mkfile(_d->generateUserDirectory(username) + separator() + "bad.dat");
}

QString UserManager::dataDirictory()
{
	return _d->dataDirectory;
}

QString UserManager::userDirectory(const QString& username)
{
	if (!exists(username)) return QString();
    return _d->generateUserDirectory(username);
}

void UserManager::setAutoLogin(const QString& password)
{
    if (_d->currentSessionsUser.isEmpty() || userDirectory(_d->currentSessionsUser).isEmpty()) return;
    QString json = "{ \"e\" : \"%1\", \"p\" : \"%2\" }";
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    wrfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME,
           Aes::encrypt(sstep, QByteArray().insert(0, json.arg(_d->currentSessionsUser, password))));
}

void UserManager::clearAutoLogin()
{
    QByteArray shredder;
    for (int i = 1048576; i--;) { shredder.append(qrand() % 250); }
    wrfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME, shredder);
    rm(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    mkfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
}

bool UserManager::hasAutoLogin()
{
    auto algdata = rdfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    return !algdata.isEmpty();
}

bool UserManager::tryAutoLogin()
{
    if (!hasAutoLogin()) return false;
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    auto algdata = rdfile(_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    auto jobj = QJsonDocument::fromJson(Aes::decrypt(sstep, algdata)).object();
    return startUserSession(jobj["e"].toString(), jobj["p"].toString());
}

QString UserManager::currentSessionsUser()
{
	return _d->currentSessionsUser;
}

QString UserManager::currentSessionsToken()
{
    return _d->currentSessionsToken;
}

QString UserManager::currentSessionsKey()
{
    return _d->currentSessionsKey;
}

QString UserManager::generateToken(const QString& username, const QString& password)
{
    return _d->generateToken(username, password);
}

bool UserManager::startUserSession(const QString& username, const QString& password)
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
        QMetaObject::invokeMethod(pW, "showProgress", Qt::QueuedConnection,
                                  Q_ARG(QString, "Starting user session"));

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

void UserManager::stopUserSession()
{
	if (_d->currentSessionsUser.isEmpty()) {
		return;
	}

	ProjectManager::stopProject();

	if (exists(_d->currentSessionsUser) && !_d->dirLocker.locked(userDirectory(_d->currentSessionsUser))) {
		if (!_d->dirLocker.lock(userDirectory(_d->currentSessionsUser), _d->currentSessionsKey)) {
			qFatal("ProjectManager : Error occurred");
		}
	}

	_d->currentSessionsUser = "";
	_d->currentSessionsKey = "";
    _d->currentSessionsToken = "";
}

