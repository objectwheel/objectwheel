#include <usermanager.h>
#include <QStandardPaths>
#include <QCoreApplication>
#include <filemanager.h>
#include <QCryptographicHash>
#include <QByteArray>
#include <dirlocker.h>
#include <projectmanager.h>
#include <splashscreen.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <aes.h>

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

UserManagerPrivate* UserManager::m_d = nullptr;

UserManager::UserManager(QObject *parent)
	: QObject(parent)
{
	if (m_d) return;
    m_d = new UserManagerPrivate(this);
}

UserManager::~UserManager()
{
    delete m_d;
}

UserManager* UserManager::instance()
{
	return m_d->parent;
}

bool UserManager::exists(const QString& username)
{
	return ::exists(m_d->generateUserDirectory(username));
}

bool UserManager::buildNewUser(const QString& username)
{
//	if (exists(username)) return false; //True code
//	return mkdir(m_d->generateUserDirectory(username));
    //FIXME:
    if (exists(username)) return false; //Bad code
    return mkfile(m_d->generateUserDirectory(username) + separator() + "bad.dat");
}

QString UserManager::dataDirictory()
{
	return m_d->dataDirectory;
}

QString UserManager::userDirectory(const QString& username)
{
	if (!exists(username)) return QString();
    return m_d->generateUserDirectory(username);
}

void UserManager::setAutoLogin(const QString& password)
{
    if (m_d->currentSessionsUser.isEmpty() || userDirectory(m_d->currentSessionsUser).isEmpty()) return;
    QString json = "{ \"e\" : \"%1\", \"p\" : \"%2\" }";
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    wrfile(m_d->dataDirectory + separator() + AUTOLOGIN_FILENAME,
           Aes::encrypt(sstep, QByteArray().insert(0, json.arg(m_d->currentSessionsUser, password))));
}

void UserManager::clearAutoLogin()
{
    QByteArray shredder;
    for (int i = 1048576; i--;) { shredder.append(qrand() % 250); }
    wrfile(m_d->dataDirectory + separator() + AUTOLOGIN_FILENAME, shredder);
    rm(m_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    mkfile(m_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
}

bool UserManager::hasAutoLogin()
{
    auto algdata = rdfile(m_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    return !algdata.isEmpty();
}

bool UserManager::tryAutoLogin()
{
    if (!hasAutoLogin()) return false;
    auto fstep = QByteArray::fromBase64(AUTOLOGIN_PROTECTOR);
    auto sstep = QCryptographicHash::hash(fstep, QCryptographicHash::Md5).toHex();
    auto algdata = rdfile(m_d->dataDirectory + separator() + AUTOLOGIN_FILENAME);
    auto jobj = QJsonDocument::fromJson(Aes::decrypt(sstep, algdata)).object();
    return startUserSession(jobj["e"].toString(), jobj["p"].toString());
}

QString UserManager::currentSessionsUser()
{
	return m_d->currentSessionsUser;
}

QString UserManager::currentSessionsToken()
{
    return m_d->currentSessionsToken;
}

QString UserManager::currentSessionsKey()
{
    return m_d->currentSessionsKey;
}

QString UserManager::generateToken(const QString& username, const QString& password)
{
    return m_d->generateToken(username, password);
}

bool UserManager::startUserSession(const QString& username, const QString& password)
{
	if (m_d->currentSessionsUser == username) {
		return true;
	}

	if (!exists(username)) {
		return false;
	}

	if (!m_d->currentSessionsUser.isEmpty()) {
		stopUserSession();
	}

    auto keyHash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Sha3_512);
    keyHash = QCryptographicHash::hash(keyHash, QCryptographicHash::Md5).toHex();
	m_d->currentSessionsUser = username;
	m_d->currentSessionsKey = keyHash;
    m_d->currentSessionsToken = m_d->generateToken(username, password);

    if (m_d->dirLocker.canUnlock(userDirectory(username), keyHash)) {
        SplashScreen::setText("Decrypting user data.. starting session");
		if (!m_d->dirLocker.unlock(userDirectory(username), keyHash)) {
			m_d->currentSessionsUser = "";
			m_d->currentSessionsKey = "";
            m_d->currentSessionsToken = "";
			return false;
		}
	}
	return true;
}

void UserManager::stopUserSession()
{
	if (m_d->currentSessionsUser.isEmpty()) {
		return;
	}

	ProjectManager::stopProject();

	if (exists(m_d->currentSessionsUser) && !m_d->dirLocker.locked(userDirectory(m_d->currentSessionsUser))) {
		if (!m_d->dirLocker.lock(userDirectory(m_d->currentSessionsUser), m_d->currentSessionsKey)) {
			qFatal("ProjectManager : Error occurred");
		}
	}

	m_d->currentSessionsUser = "";
	m_d->currentSessionsKey = "";
    m_d->currentSessionsToken = "";
}

