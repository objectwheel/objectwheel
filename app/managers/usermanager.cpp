#include <usermanager.h>
#include <applicationcore.h>
#include <saveutils.h>
#include <utilityfunctions.h>
#include <registrationapimanager.h>
#include <hashfactory.h>

#include <QDir>
#include <QDateTime>

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
    static const QString& userBaseDir = ApplicationCore::appDataLocation() + QStringLiteral("/users");
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

QString UserManager::hashPassword(const QString& password)
{
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha3_512).toHex();
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

bool UserManager::isLoggedIn()
{
    return !s_email.isEmpty();
}

void UserManager::login(const QString& email, const QString& password)
{
    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        qWarning("UserManager: Incorrect email format");
        return;
    }

    if (!UtilityFunctions::isPasswordFormatCorrect(password)) {
        qWarning("UserManager: Incorrect password format");
        return;
    }

    if (isLoggedIn()) {
        qWarning("UserManager: Already logged in");
        return;
    }

    s_emailCache = email;
    s_passwordCache = hashPassword(password);

    RegistrationApiManager::login(s_emailCache, s_passwordCache);
}

void UserManager::loginOffline(const QString& email, const QString& hash)
{
    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        qWarning("UserManager: Incorrect email format");
        return;
    }

    if (hash.size() != 128) {
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
    s_passwordCache = hash;

    if (UtilityFunctions::testPassword(hash.toUtf8(), SaveUtils::userPassword(dir(email))))
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
        SaveUtils::makeUserMetaFile(userDir);
    }

    s_plan = PlanManager::Free;
    s_email = s_emailCache;
    s_password = s_passwordCache;

    UtilityFunctions::cleanSensitiveInformation(s_emailCache);
    UtilityFunctions::cleanSensitiveInformation(s_passwordCache);

    // userRecord: icon, timestamp, plan, first, last, country, company, title, phone
    if (!userInfo.isEmpty()) {
        SaveUtils::setProperty(userDir, SaveUtils::UserRegistrationDate, userInfo.at(1));
        SaveUtils::setProperty(userDir, SaveUtils::UserPlan, userInfo.at(2));
        SaveUtils::setProperty(userDir, SaveUtils::UserFirst, userInfo.at(3));
        SaveUtils::setProperty(userDir, SaveUtils::UserLast, userInfo.at(4));
        SaveUtils::setProperty(userDir, SaveUtils::UserCountry, userInfo.at(5));
        SaveUtils::setProperty(userDir, SaveUtils::UserCompany, userInfo.at(6));
        SaveUtils::setProperty(userDir, SaveUtils::UserTitle, userInfo.at(7));
        SaveUtils::setProperty(userDir, SaveUtils::UserPhone, userInfo.at(8));
        SaveUtils::setProperty(userDir, SaveUtils::UserLastOnlineDate, QDateTime::currentDateTime());
        SaveUtils::setProperty(userDir, SaveUtils::UserEmail, s_email);
        SaveUtils::setProperty(userDir, SaveUtils::UserIcon, userInfo.at(0));
        SaveUtils::setProperty(userDir, SaveUtils::UserPassword, UtilityFunctions::generatePasswordHash(s_password.toUtf8()));
        s_plan = userInfo.at(1).value<PlanManager::Plans>();
    } else {
        s_plan = static_cast<PlanManager::Plans>(SaveUtils::userPlan(userDir));
    }

    emit loggedIn();
}
