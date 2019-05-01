#include <projectmanager.h>
#include <usermanager.h>
#include <savemanager.h>
#include <hashfactory.h>
#include <saveutils.h>
#include <controlpreviewingmanager.h>
#include <projectexposingmanager.h>
#include <toolmanager.h>
#include <documentmanager.h>
#include <zipasync.h>
#include <filesystemutils.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

ProjectManager* ProjectManager::s_instance = nullptr;
QString ProjectManager::s_currentUid;

ProjectManager* ProjectManager::instance()
{
    return s_instance;
}

bool ProjectManager::newProject(int templateNumber, const QString& name, const QString& description,
                                const QDateTime& crDate)
{
    const auto& udir = UserManager::dir();

    if (udir.isEmpty()
            || name.isEmpty()
            || description.isEmpty()
            || !crDate.isValid()) {
        return false;
    }

    const QString& pdir = udir + '/' + HashFactory::generate();
    const QString& uid = HashFactory::generate();

    if (!QDir(pdir).mkpath("."))
        return false;

    SaveUtils::makeProjectMetaFile(pdir);
    SaveUtils::setProperty(pdir, SaveUtils::ProjectName, name);
    SaveUtils::setProperty(pdir, SaveUtils::ProjectDescription, description);
    SaveUtils::setProperty(pdir, SaveUtils::ProjectCreationDate, crDate);
    SaveUtils::setProperty(pdir, SaveUtils::ProjectUid, uid);

    if (!SaveManager::initProject(pdir, templateNumber))
        return false;

    updateSize(uid);

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

bool ProjectManager::importProject(const QString &filePath, QString* uid)
{
    const auto& udir = UserManager::dir();
    const auto& pdir = udir + '/' + HashFactory::generate();

    if (filePath.isEmpty() || udir.isEmpty())
        return false;

    if (!QDir(pdir).mkpath(".") || !ZipAsync::unzipSync(filePath, pdir))
        return false;

    *uid = HashFactory::generate();

    SaveUtils::setProperty(pdir, SaveUtils::ProjectUid, *uid);
    SaveUtils::regenerateUids(SaveUtils::toDesignsDir(pdir));

    return true;
}

QString ProjectManager::dir(const QString& uid)
{
    if (uid.isEmpty()) {
        qWarning("ProjectManager: Caution, empty uid.");
        return QStringLiteral("");
    }

    const QString& userDirectory = UserManager::dir();

    if (userDirectory.isEmpty()) {
        qWarning("ProjectManager: Caution, empty user dir.");
        return QStringLiteral("");
    }

    for (const QString& projectDirName
         : QDir(userDirectory).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& projectDir = userDirectory + '/' + projectDirName;
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
    return s_currentUid;
}

QStringList ProjectManager::projects()
{
    QStringList uids;
    const auto& udir = UserManager::dir();

    if (udir.isEmpty())
        return uids;

    for (const auto& dir : QDir(udir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const auto& p = udir + '/' + dir;
        if (SaveUtils::isProjectValid(p))
            uids << SaveUtils::projectUid(p);
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

    if (!ProjectManager::uid().isEmpty())
        stop();

    s_currentUid = uid;

    ToolManager::exposeTools();
    ProjectExposingManager::exposeProject();
    ControlPreviewingManager::scheduleInit();
    DocumentManager::updateProjectInfo();
    updateLastModification(s_currentUid);

    emit instance()->started();

    return true;
}

void ProjectManager::stop()
{
    ControlPreviewingManager::scheduleTerminate();
    updateSize(s_currentUid);
    s_currentUid = "";
    emit instance()->stopped();
}
