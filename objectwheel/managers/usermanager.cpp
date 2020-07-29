#include <usermanager.h>
#include <applicationcore.h>
#include <saveutils.h>
#include <registrationapimanager.h>
#include <hashfactory.h>
#include <utilityfunctions.h>
#include <QDir>

UserManager* UserManager::s_instance = nullptr;
PlanManager::Plans UserManager::s_plan = PlanManager::Free;
QString UserManager::s_email;
QString UserManager::s_password;
QString UserManager::s_emailCache;
QString UserManager::s_passwordCache;

UserManager::UserManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::loginSuccessful,
            this, &UserManager::onLoginSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::loginFailure,
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

PlanManager::Plans UserManager::plan()
{
    return s_plan;
}

QString UserManager::email()
{
    return s_email;
}

QString UserManager::password()
{
    return s_password;
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

    s_plan = PlanManager::Free;
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

    RegistrationApiManager::login(email, password);
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
        s_instance->onLoginSuccessful(QVariantList());
    else
        s_instance->onLoginFailure();
}

void UserManager::onLoginFailure()
{
    UtilityFunctions::cleanSensitiveInformation(s_emailCache);
    UtilityFunctions::cleanSensitiveInformation(s_passwordCache);
    emit loginFailed();
}

void UserManager::onLoginSuccessful(const QVariantList& userInfo)
{
    QString userDir = dir(s_emailCache);

    if (userDir.isEmpty()) {
        userDir = baseDirectory() + '/' + HashFactory::generate();
        if (!QDir(userDir).mkpath(".")) {
            qWarning("UserManager: Cannot create a user directory");
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

    if (!userInfo.isEmpty()) {
        s_plan = userInfo.at(0).value<PlanManager::Plans>();
        SaveUtils::setProperty(userDir, SaveUtils::UserPlan, quint32(s_plan));
        SaveUtils::setProperty(userDir, SaveUtils::UserLastOnlineDate, QDateTime::currentDateTime());
        SaveUtils::setProperty(userDir, SaveUtils::UserEmail, s_email);
        SaveUtils::setProperty(userDir, SaveUtils::UserPassword, UtilityFunctions::generatePasswordHash(s_password.toUtf8()));
    } else {
        s_plan = static_cast<PlanManager::Plans>(SaveUtils::userPlan(userDir));
    }

    emit loggedIn();
}
