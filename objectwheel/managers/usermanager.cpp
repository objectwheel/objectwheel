#include <usermanager.h>
#include <applicationcore.h>
#include <saveutils.h>
#include <apimanager.h>
#include <hashfactory.h>
#include <utilityfunctions.h>
#include <QDir>

UserManager* UserManager::s_instance = nullptr;
qint64 UserManager::s_plan = 0;
QString UserManager::s_email;
QString UserManager::s_password;
QString UserManager::s_emailCache;
QString UserManager::s_passwordCache;

UserManager::UserManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    connect(ApiManager::instance(), &ApiManager::loginSuccessful,
            this, [this] (qint64 plan) { onLoginSuccessful(plan, true); });
    connect(ApiManager::instance(), &ApiManager::loginFailure,
            this, &UserManager::onLoginFailure);
}

UserManager::~UserManager()
{
    s_instance = nullptr;
}

UserManager* UserManager::instance()
{
    return s_instance;
}

QString UserManager::baseDirectory()
{
    static const QString& userBaseDir = ApplicationCore::appDataPath() + QStringLiteral("/Users");
    return userBaseDir;
}

QStringList UserManager::userDirs()
{
    QStringList dirs;
    for (const QString& userDirName
         : QDir(baseDirectory()).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& userDir = baseDirectory() + '/' + userDirName;
        if (SaveUtils::isUserValid(userDir))
            dirs.append(userDir);
    }
    return dirs;
}

QStringList UserManager::users()
{    
    QStringList userList;
    for (const QString& userDir : userDirs())
        userList.append(SaveUtils::userEmail(userDir));
    return userList;
}

QString UserManager::dir(const QString& email)
{
    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        qWarning("UserManager: User dir requested with incorrect email format");
        return QString();
    }

    for (const QString& userDir : userDirs()) {
        if (SaveUtils::userEmail(userDir) == email)
            return userDir;
    }

    return QString();
}

QString UserManager::email()
{
    return s_email;
}

QString UserManager::password()
{
    return s_password;
}

qint64 UserManager::plan()
{
    return s_plan;
}

bool UserManager::isLoggedIn()
{
    return !s_email.isEmpty();
}

bool UserManager::hasLocalData(const QString& email)
{
    return users().contains(email);
}

void UserManager::logout()
{
    if (!isLoggedIn())
        return;

    emit s_instance->aboutToLogout();

    s_plan = 0;
    UtilityFunctions::cleanSensitiveInformation(s_email);
    UtilityFunctions::cleanSensitiveInformation(s_password);

    emit s_instance->loggedOut();
}

void UserManager::login(const QString& email, const QString& password)
{
    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        qWarning("UserManager: Incorrect email format");
        return;
    }

    if (!UtilityFunctions::isPasswordHashFormatCorrect(password)) {
        qWarning("UserManager: Incorrect password format");
        return;
    }

    if (isLoggedIn()) {
        qWarning("UserManager: Already logged in");
        return;
    }

    s_emailCache = email;
    s_passwordCache = password;

    ApiManager::login(email, password);
}

void UserManager::loginOffline(const QString& email, const QString& password)
{
    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        qWarning("UserManager: Incorrect email format");
        return;
    }

    if (!UtilityFunctions::isPasswordHashFormatCorrect(password)) {
        qWarning("UserManager: Incorrect hash format");
        return;
    }

    if (isLoggedIn()) {
        qWarning("UserManager: Already logged in");
        return;
    }

    if (!hasLocalData(email)) {
        qWarning("UserManager: Cannot login offline, no local data");
        return;
    }

    s_emailCache = email;
    s_passwordCache = password;

    if (UtilityFunctions::testPassword(password.toUtf8(), SaveUtils::userPassword(dir(email))))
        s_instance->onLoginSuccessful(0, false);
    else
        s_instance->onLoginFailure();
}

void UserManager::onLoginFailure()
{
    UtilityFunctions::cleanSensitiveInformation(s_emailCache);
    UtilityFunctions::cleanSensitiveInformation(s_passwordCache);
    emit loginFailed();
}

void UserManager::onLoginSuccessful(qint64 plan, bool online)
{
    QString userDir = dir(s_emailCache);

    if (userDir.isEmpty()) {
        userDir = baseDirectory() + '/' + HashFactory::generate();
        if (!QDir(userDir).mkpath(".")) {
            qWarning("UserManager: Cannot establish a user directory");
            UtilityFunctions::cleanSensitiveInformation(s_emailCache);
            UtilityFunctions::cleanSensitiveInformation(s_passwordCache);
            return;
        }
        SaveUtils::initUserMeta(userDir);
    }

    s_email = s_emailCache;
    s_password = s_passwordCache;

    UtilityFunctions::cleanSensitiveInformation(s_emailCache);
    UtilityFunctions::cleanSensitiveInformation(s_passwordCache);

    if (online) {
        s_plan = plan;
        SaveUtils::setProperty(userDir, SaveUtils::UserPlan, s_plan);
        SaveUtils::setProperty(userDir, SaveUtils::UserEmail, s_email);
        SaveUtils::setProperty(userDir, SaveUtils::UserPassword, UtilityFunctions::generatePasswordHash(s_password.toUtf8()));
    } else {
        s_plan = SaveUtils::userPlan(userDir);
    }

    emit loggedIn();
}
