#include <projectmanager.h>
#include <usermanager.h>
#include <savemanager.h>
#include <hashfactory.h>
#include <saveutils.h>
#include <controlrenderingmanager.h>
#include <projectexposingmanager.h>
#include <documentmanager.h>
#include <zipasync.h>
#include <filesystemutils.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

ProjectManager* ProjectManager::s_instance = nullptr;
QString ProjectManager::s_uid;

ProjectManager* ProjectManager::instance()
{
    return s_instance;
}

bool ProjectManager::initProject(int templateNumber, const QString& name, const QString& description,
                                 const QDateTime& crDate)
{
    if (!UserManager::isLoggedIn()
            || name.isEmpty()
            || description.isEmpty()
            || !crDate.isValid()) {
        return false;
    }

    const QString& projectDir = SaveUtils::toUserProjectsDir(UserManager::dir()) + '/' + HashFactory::generate();

    if (!QDir(projectDir).mkpath("."))
        return false;

    // Extract "designs" and "imports" directories
    if (!ZipAsync::unzipSync(":/templates/template" + QString::number(templateNumber) + ".zip", projectDir)) {
        qWarning("WARNING: Unzipping project template unsuccessful");
        return false;
    }

    SaveUtils::initProjectMeta(projectDir);
    SaveUtils::setProperty(projectDir, SaveUtils::ProjectName, name);
    SaveUtils::setProperty(projectDir, SaveUtils::ProjectDescription, description);
    SaveUtils::setProperty(projectDir, SaveUtils::ProjectCreationDate, crDate);
    SaveUtils::regenerateUids(SaveUtils::toProjectDesignsDir(projectDir));

    updateSize(SaveUtils::projectUid(projectDir));

    return true;
}

ProjectManager::ProjectManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ProjectManager::~ProjectManager()
{
    s_instance = nullptr;
}

QStringList ProjectManager::projectNames()
{
    QStringList names;
    const auto& uids = projects();

    if (uids.isEmpty())
        return names;

    for (const auto& uid : uids)
        names << SaveUtils::projectName(dir(uid));

    return names;
}

void ProjectManager::changeName(const QString& uid, const QString& name)
{
    const auto& projectDir = dir(uid);

    if (projectDir.isEmpty())
        return;

    SaveUtils::setProperty(projectDir, SaveUtils::ProjectName, name);
}

void ProjectManager::changeDescription(const QString& uid, const QString& desc)
{
    const auto& projectDir = dir(uid);

    if (projectDir.isEmpty())
        return;

    SaveUtils::setProperty(projectDir, SaveUtils::ProjectDescription, desc);
}

bool ProjectManager::exportProject(const QString& uid, const QString& filePath)
{
    const auto& projectDir = dir(uid);

    if (projectDir.isEmpty() || filePath.isEmpty())
        return false;

    return ZipAsync::zipSync(projectDir, filePath);
}

bool ProjectManager::importProject(const QString& filePath, QString* uid)
{
    if (filePath.isEmpty())
        return false;

    if (!UserManager::isLoggedIn())
        return false;

    const QString& projectDir = SaveUtils::toUserProjectsDir(UserManager::dir()) + '/' + HashFactory::generate();

    if (!QDir(projectDir).mkpath(".") || !ZipAsync::unzipSync(filePath, projectDir))
        return false;

    *uid = HashFactory::generate();

    SaveUtils::setProperty(projectDir, SaveUtils::ProjectUid, *uid);
    SaveUtils::regenerateUids(SaveUtils::toProjectDesignsDir(projectDir));

    return true;
}

QString ProjectManager::dir(const QString& uid)
{
    if (uid.isEmpty()) {
        qWarning("ProjectManager: Caution, empty uid.");
        return QStringLiteral("");
    }

    if (!UserManager::isLoggedIn()) {
        qWarning("ProjectManager: Caution, user is not logged in yet.");
        return QStringLiteral("");
    }

    const QString& projectsDirectory = SaveUtils::toUserProjectsDir(UserManager::dir());

    for (const QString& projectDirName
         : QDir(projectsDirectory).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& projectDir = projectsDirectory + '/' + projectDirName;
        if (SaveUtils::isProjectValid(projectDir) && SaveUtils::projectUid(projectDir) == uid)
            return projectDir;
    }

    return QStringLiteral("");
}

QString ProjectManager::name(const QString& uid)
{
    return SaveUtils::projectName(dir(uid));
}

QString ProjectManager::description(const QString& uid)
{
    return SaveUtils::projectDescription(dir(uid));
}

QDateTime ProjectManager::crDate(const QString& uid)
{
    return SaveUtils::projectCreationDate(dir(uid));
}

QDateTime ProjectManager::mfDate(const QString& uid)
{
    return SaveUtils::projectModificationDate(dir(uid));
}

qint64 ProjectManager::size(const QString& uid)
{
    return SaveUtils::projectSize(dir(uid));
}

QString ProjectManager::uid()
{
    return s_uid;
}

QStringList ProjectManager::projects()
{
    QStringList uids;

    if (!UserManager::isLoggedIn()) {
        qWarning("ProjectManager: User not logged in yet");
        return uids;
    }

    const QString& projectsDir = SaveUtils::toUserProjectsDir(UserManager::dir());
    for (const QString& projectDirName : QDir(projectsDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& projectDir = projectsDir + '/' + projectDirName;
        if (SaveUtils::isProjectValid(projectDir))
            uids << SaveUtils::projectUid(projectDir);
    }

    return uids;
}

void ProjectManager::updateSize(const QString& uid) // WARNING: Severe performance issues
{
    const auto& projectDir = dir(uid);
    if (projectDir.isEmpty())
        return;

    SaveUtils::setProperty(projectDir, SaveUtils::ProjectSize,
                           FileSystemUtils::directorySize(projectDir));
}

void ProjectManager::updateLastModification(const QString& uid)
{
    const auto& projectDir = dir(uid);
    if (projectDir.isEmpty())
        return;

    SaveUtils::setProperty(projectDir, SaveUtils::ProjectModificationDate, 0);
}

bool ProjectManager::start(const QString& uid)
{
    if (ProjectManager::uid() == uid)
        return true;

    const auto& projectDir = dir(uid);

    if (projectDir.isEmpty())
        return false;

    if (ProjectManager::isStarted())
        stop();

    s_uid = uid;

    DocumentManager::updateProjectInfo(dir());
    // Must be executed before running render engine
    // Because it might fix broken id and indexes
    ProjectExposingManager::exposeProject();
    ControlRenderingManager::start();
    updateLastModification(s_uid);

    emit instance()->started();

    return true;
}

void ProjectManager::stop()
{
    ControlRenderingManager::terminate();
    ProjectExposingManager::removeProject();
    DocumentManager::removeProjectInfo();
    updateSize(s_uid);
    s_uid = "";
    emit instance()->stopped();
}

bool ProjectManager::isStarted()
{
    return !s_uid.isEmpty();
}
