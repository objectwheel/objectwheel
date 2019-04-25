#include <projectmanager.h>
#include <usermanager.h>
#include <savemanager.h>
#include <filemanager.h>
#include <hashfactory.h>
#include <saveutils.h>
#include <controlpreviewingmanager.h>
#include <projectexposingmanager.h>
#include <toolmanager.h>
#include <documentmanager.h>
#include <zipasync.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

namespace {

QString byteString(const qint64 size)
{
    QString ret;
    float kb = 1024.0f;
    float mb = 1048576.0f;
    float gb = 1073741824.0f;

    if (size < kb) {
        ret = QString::number(size);
        ret += " Bytes";
    } else if (size < mb) {
        ret = QString::number(size / kb, 'f', 1);
        ret += " KB";
    } else if (size < gb) {
        ret = QString::number(size / mb, 'f', 1);
        ret += " MB";
    } else {
        ret = QString::number(size / gb, 'f', 2);
        ret += " GB";
    }

    return ret;
}

void setProperty(const QString& rootPath, const QString& property, const QVariant& value)
{
    auto jobj = QJsonDocument::fromJson(rdfile(rootPath +separator() + FILE_PROJECT)).object();
    jobj[property] = value.toJsonValue();
    if (property != PTAG_MFDATE)
        jobj[PTAG_MFDATE] = ProjectManager::currentDbTime();
    const auto& data = QJsonDocument(jobj).toJson();
    wrfile(rootPath + separator() + FILE_PROJECT, data);
}

bool isOwprj(const QString& rootPath)
{
    const auto& jobj = QJsonDocument::fromJson(
                rdfile(
                    rootPath +
                    separator() +
                    FILE_PROJECT
                    )
                ).object();

    return (jobj.value(PTAG_OWPRJ_SIGN).toString() == SIGN_OWPRJ);
}

QString uid(const QString& rootPath)
{
    const auto& jobj = QJsonDocument::fromJson(
                rdfile(
                    rootPath +
                    separator() +
                    FILE_PROJECT
                    )
                ).object();

    return jobj.value(PTAG_UID).toString();
}

QString dir(const QString& uid)
{
    QString pdir;
    const auto& udir = UserManager::dir();

    if (udir.isEmpty() || uid.isEmpty())
        return pdir;

    for (const auto& dir : lsdir(udir)) {
        const auto& p = udir + separator() + dir;

        if (!isOwprj(p))
            continue;

        auto h = ::uid(p);

        if (uid == h) {
            pdir = p;
            break;
        }
    }

    return pdir;
}

QVariant property(const QString& uid, const QString& property)
{
    const auto& dir = ::dir(uid);

    if (dir.isEmpty())
        return QVariant();

    const auto& jobj = QJsonDocument::fromJson(
                rdfile(
                    dir +
                    separator() +
                    FILE_PROJECT
                    )
                ).object();

    return jobj.value(property).toVariant();
}
}

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
                                const QString& owner, const QString& crDate)
{
    const auto& udir = UserManager::dir();

    if (udir.isEmpty()
            || name.isEmpty()
            || description.isEmpty()
            || owner.isEmpty()
            || crDate.isEmpty()) {
        return false;
    }

    const auto& pdir = udir + separator() + HashFactory::generate();
    const auto& uid = HashFactory::generate();

    QJsonObject jobj;
    jobj.insert(PTAG_NAME, name);
    jobj.insert(PTAG_DESCRIPTION, description);
    jobj.insert(PTAG_OWNER, owner);
    jobj.insert(PTAG_CRDATE, crDate);
    jobj.insert(PTAG_MFDATE, crDate);
    jobj.insert(PTAG_OWPRJ_SIGN, SIGN_OWPRJ);
    jobj.insert(PTAG_UID, uid);

    const auto& data = QJsonDocument(jobj).toJson();

    if (!mkdir(pdir))
        return false;

    if (wrfile(pdir + separator() + FILE_PROJECT, data) <= 0)
        return false;

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
        names << ::property(uid, PTAG_NAME).toString();

    return names;
}

void ProjectManager::changeName(const QString& uid, const QString& name)
{
    const auto& dir = ::dir(uid);

    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_NAME, name);
}

void ProjectManager::changeDescription(const QString& uid, const QString& desc)
{
    const auto& dir = ::dir(uid);

    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_DESCRIPTION, desc);
}

bool ProjectManager::exportProject(const QString& uid, const QString& filePath)
{
    const auto& dir = ::dir(uid);

    if (dir.isEmpty() || filePath.isEmpty())
        return false;

    return ZipAsync::zipSync(dir, filePath);
}

bool ProjectManager::importProject(const QString &filePath, QString* uid)
{
    const auto& udir = UserManager::dir();
    const auto& pdir = udir + separator() + HashFactory::generate();

    if (filePath.isEmpty() || udir.isEmpty())
        return false;

    if (!mkdir(pdir) || !ZipAsync::unzipSync(filePath, pdir))
        return false;

    *uid = HashFactory::generate();
    ::setProperty(pdir, PTAG_UID, *uid);

    SaveUtils::regenerateUids(pdir + separator() + DIR_OWDB);

    return true;
}

QString ProjectManager::dir(const QString& uid)
{
    return ::dir(uid);
}

QString ProjectManager::name(const QString& uid)
{
    return ::property(uid, PTAG_NAME).toString();
}

QString ProjectManager::description(const QString& uid)
{
    return ::property(uid, PTAG_DESCRIPTION).toString();
}

QString ProjectManager::owner(const QString& uid)
{
    return ::property(uid, PTAG_OWNER).toString();
}

QString ProjectManager::crDate(const QString& uid)
{
    return ::property(uid, PTAG_CRDATE).toString();
}

QString ProjectManager::mfDate(const QString& uid)
{
    return ::property(uid, PTAG_MFDATE).toString();
}

QString ProjectManager::size(const QString& uid)
{
    return ::property(uid, PTAG_SIZE).toString();
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

    for (const auto& dir : lsdir(udir)) {
        const auto& p = udir + separator() + dir;
        if (isOwprj(p))
            uids << ::uid(p);
    }

    return uids;
}

void ProjectManager::updateSize(const QString& uid)
{
    const auto& dir = ::dir(uid);
    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_SIZE, byteString(dsize(dir)));
}

void ProjectManager::updateLastModification(const QString& uid)
{
    const auto& dir = ::dir(uid);
    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_MFDATE, ProjectManager::currentDbTime());
}

bool ProjectManager::start(const QString& uid)
{
    if (ProjectManager::uid() == uid)
        return true;

    const auto& dir = ::dir(uid);

    if (dir.isEmpty())
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
