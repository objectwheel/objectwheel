#include <usermanager.h>
#include <applicationcore.h>
#include <saveutils.h>
#include <utilityfunctions.h>
#include <servermanager.h>
#include <registrationapimanager.h>
#include <hashfactory.h>
#include <QDir>

UserManager* UserManager::s_instance = nullptr;
UserManager::Plans UserManager::s_plan = UserManager::Free;
QString UserManager::s_email;
QString UserManager::s_password;

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

UserManager::Plans UserManager::plan()
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

void UserManager::logout()
{
    if (!isLoggedIn())
        return;

    emit s_instance->aboutToLogout();

    s_plan = Free;
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

    if (ServerManager::isConnected()) {
        RegistrationApiManager::login(email, password);
    } else {
        const QString& userDir = dir(email);

        if (userDir.isEmpty()) {
            qWarning("UserManager: Cannot log in offline with a non-existent user account");
            return;
        }

        if (UtilityFunctions::testPassword(password.toUtf8(), SaveUtils::userHash(userDir))) {
            s_emailCache = email;
            s_passwordCache = password;
            s_instance->onLoginSuccessful({});
        } else {
            s_instance->onLoginFailure();
        }
    }
}

void UserManager::onLoginFailure()
{
    emit loginFailed();
}

void UserManager::onLoginSuccessful(const QVariantList& userInfo)
{
    // userRecord: timestamp, plan, first, last, country, company, title, phone

    QString userDir = dir(s_emailCache);

    if (userDir.isEmpty()) {
        userDir = baseDirectory() + '/' + HashFactory::generate();
        if (!QDir(userDir).mkpath(".")) {
            qWarning("UserManager: Cannot create user directory");
            UtilityFunctions::cleanSensitiveInformation(s_emailCache);
            UtilityFunctions::cleanSensitiveInformation(s_passwordCache);
            return;
        }
        SaveUtils::makeUserMetaFile(userDir);
    }

    s_email = s_emailCache;
    s_password = QCryptographicHash::hash(s_passwordCache, QCryptographicHash::Sha3_512).toHex();

    UtilityFunctions::cleanSensitiveInformation(s_emailCache);
    UtilityFunctions::cleanSensitiveInformation(s_passwordCache);

    emit loggedIn();
}
