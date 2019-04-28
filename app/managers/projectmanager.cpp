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
#include <utilityfunctions.h>
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

QString ProjectManager::currentDbTime()
{
    return QDateTime::currentDateTime().toString(Qt::ISODate);
}

QString ProjectManager::currentUiTime()
{
    return QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate);
}

QString ProjectManager::toDbTime(const QString& uiTime)
{
    return QDateTime::fromString(uiTime, Qt::SystemLocaleLongDate).toString(Qt::ISODate);
}

QString ProjectManager::toUiTime(const QString& dbTime)
{
    return QDateTime::fromString(dbTime, Qt::ISODate).toString(Qt::SystemLocaleLongDate);
}

QDateTime ProjectManager::fromDb(const QString& dbTime)
{
    return QDateTime::fromString(dbTime, Qt::ISODate);
}

QDateTime ProjectManager::fromUi(const QString& uiTime)
{
    return QDateTime::fromString(uiTime, Qt::SystemLocaleLongDate);
}

bool ProjectManager::newProject(int templateNumber, const QString& name, const QString& description,
                                const QString& crDate)
{
    const auto& udir = UserManager::dir();

    if (udir.isEmpty()
            || name.isEmpty()
            || description.isEmpty()
            || crDate.isEmpty()) {
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
    SaveUtils::setProperty(pdir, SaveUtils::ProjectModificationDate, crDate);
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
    QString found;
    const QString& userDirectory = UserManager::dir();

    if (userDirectory.isEmpty() || uid.isEmpty())
        return found;

    for (const QString& projectName : QDir(userDirectory).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& projectDir = userDirectory + '/' + projectName;

        if (!SaveUtils::isProjectValid(projectDir))
            continue;

        QString projectUid = SaveUtils::projectUid(projectDir);

        if (uid == projectUid) {
            found = projectDir;
            break;
        }
    }

    return found;
}

QString ProjectManager::name(const QString& uid)
{
    return SaveUtils::projectName(dir(uid));
}

QString ProjectManager::description(const QString& uid)
{
    return SaveUtils::projectDescription(dir(uid));
}

QString ProjectManager::crDate(const QString& uid)
{
    return SaveUtils::projectCreationDate(dir(uid));
}

QString ProjectManager::mfDate(const QString& uid)
{
    return SaveUtils::projectModificationDate(dir(uid));
}

QString ProjectManager::size(const QString& uid)
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

void ProjectManager::updateSize(const QString& uid)
{
    const auto& projectDir = dir(uid);
    if (projectDir.isEmpty())
        return;


    SaveUtils::setProperty(projectDir, SaveUtils::ProjectSize,
        UtilityFunctions::toPrettyBytesString(FileSystemUtils::directorySize(projectDir), true));
}

void ProjectManager::updateLastModification(const QString& uid)
{
    const auto& projectDir = dir(uid);
    if (projectDir.isEmpty())
        return;

    SaveUtils::setProperty(projectDir, SaveUtils::ProjectModificationDate, ProjectManager::currentDbTime());
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
