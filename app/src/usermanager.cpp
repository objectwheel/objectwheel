#include <usermanager.h>
#include <QStandardPaths>
#include <QCoreApplication>
#include <filemanager.h>
#include <QCryptographicHash>
#include <QByteArray>
#include <dirlocker.h>
#include <projectmanager.h>

class UserManagerPrivate
{
	public:
		UserManagerPrivate(UserManager* uparent);
		QString defaultDataDirectory() const;
		QString generateUserDirectory(const QString& username) const;

	public:
		UserManager* parent = nullptr;
		QString dataDirectory;
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
	if (exists(username)) return false;
	return mkdir(m_d->generateUserDirectory(username));
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

QString UserManager::currentSessionsUser()
{
	return m_d->currentSessionsUser;
}

QString UserManager::currentSessionsKey()
{
	return m_d->currentSessionsKey;
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

	auto keyHash = QCryptographicHash::hash(QByteArray().insert(0, password), QCryptographicHash::Md5).toHex();
	m_d->currentSessionsUser = username;
	m_d->currentSessionsKey = keyHash;

	if (m_d->dirLocker.canUnlock(userDirectory(username), keyHash)) {
		if (!m_d->dirLocker.unlock(userDirectory(username), keyHash)) {
			m_d->currentSessionsUser = "";
			m_d->currentSessionsKey = "";
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
}

