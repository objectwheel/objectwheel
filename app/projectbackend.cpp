#include <projectbackend.h>
#include <userbackend.h>
#include <savebackend.h>
#include <filemanager.h>
#include <zipper.h>
#include <hashfactory.h>
#include <saveutils.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

namespace {

int biggestDir(const QString& basePath)
{
    int num = 0;
    for (auto dir : lsdir(basePath))
        if (dir.toInt() > num)
            num = dir.toInt();
    return num;
}

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

void setProperty( const QString& rootPath, const QString& property, const QVariant& value)
{
    auto jobj = QJsonDocument::fromJson(rdfile(rootPath +separator() + FILE_PROJECT)).object();

    jobj[property] = value.toJsonValue();
    jobj[PTAG_MFDATE] = QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ");

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

QString hash(const QString& rootPath)
{
    const auto& jobj = QJsonDocument::fromJson(
                rdfile(
                    rootPath +
                    separator() +
                    FILE_PROJECT
                    )
                ).object();

    return jobj.value(PTAG_HASH).toString();
}

QString dir(const QString& hash)
{
    QString pdir;
    const auto& udir = UserBackend::instance()->dir();

    if (udir.isEmpty() || hash.isEmpty())
        return pdir;

    for (const auto& dir : lsdir(udir)) {
        const auto& p = udir + separator() + dir;

        if (!isOwprj(p))
            continue;

        auto h = ::hash(p);

        if (hash == h) {
            pdir = p;
            break;
        }
    }

    return pdir;
}

QVariant property(const QString& hash, const QString& property)
{
    const auto& dir = ::dir(hash);

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

ProjectBackend* ProjectBackend::s_instance = nullptr;
QString ProjectBackend::s_currentHash;

ProjectBackend* ProjectBackend::instance()
{
    return s_instance;
}

bool ProjectBackend::newProject(int templateNumber, const QString& name, const QString& description,
                                const QString& owner, const QString& crDate, const QString& size)
{
    const auto& udir = UserBackend::instance()->dir();

    if (udir.isEmpty()
            || name.isEmpty()
            || description.isEmpty()
            || owner.isEmpty()
            || crDate.isEmpty()
            || size.isEmpty()) {
        return false;
    }

    const auto& pdir = udir + separator() + QString::number(biggestDir(udir) + 1);

    QJsonObject jobj;
    jobj.insert(PTAG_PROJECTNAME, name);
    jobj.insert(PTAG_DESCRIPTION, description);
    jobj.insert(PTAG_OWNER, owner);
    jobj.insert(PTAG_CRDATE, crDate);
    jobj.insert(PTAG_MFDATE, crDate);
    jobj.insert(PTAG_SIZE, size);
    jobj.insert(PTAG_OWPRJ_SIGN, SIGN_OWPRJ);
    jobj.insert(PTAG_HASH, HashFactory::generate());

    const auto& data = QJsonDocument(jobj).toJson();

    if (!mkdir(pdir))
        return false;

    if (wrfile(pdir + separator() + FILE_PROJECT, data) <= 0)
        return false;

    if (!SaveBackend::instance()->initProject(pdir, templateNumber))
        return false;

    return true;
}

ProjectBackend::ProjectBackend(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ProjectBackend::~ProjectBackend()
{
    s_instance = nullptr;
}

QStringList ProjectBackend::projectNames()
{
    QStringList names;
    const auto& hashes = projects();

    if (hashes.isEmpty())
        return names;

    for (const auto& hash : hashes)
        names << ::property(hash, PTAG_PROJECTNAME).toString();

    return names;
}

void ProjectBackend::changeName(const QString& hash, const QString& name)
{
    const auto& dir = ::dir(hash);

    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_PROJECTNAME, name);
}

void ProjectBackend::changeDescription(const QString& hash, const QString& desc)
{
    const auto& dir = ::dir(hash);

    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_DESCRIPTION, desc);
}

bool ProjectBackend::exportProject(const QString& hash, const QString& filePath)
{
    const auto& dir = ::dir(hash);

    if (dir.isEmpty() || filePath.isEmpty())
        return false;

    return Zipper::compressDir(dir, filePath);
}

bool ProjectBackend::importProject(const QString &filePath)
{
    const auto& data = rdfile(filePath);
    const auto& udir = UserBackend::instance()->dir();
    const auto& pdir = udir + separator() +
            QString::number(biggestDir(udir) + 1);

    if (data.isEmpty() || udir.isEmpty())
        return false;

    if (!mkdir(pdir) || !Zipper::extractZip(data, pdir))
        return false;

    ::setProperty(pdir, PTAG_HASH, HashFactory::generate());

    SaveUtils::recalculateUids(pdir + separator() + DIR_OWDB);

    return true;
}

QString ProjectBackend::dir(const QString& hash)
{
    return ::dir(hash);
}

QString ProjectBackend::name(const QString& hash)
{
    return ::property(hash, PTAG_PROJECTNAME).toString();
}

QString ProjectBackend::description(const QString& hash)
{
    return ::property(hash, PTAG_DESCRIPTION).toString();
}

QString ProjectBackend::owner(const QString& hash)
{
    return ::property(hash, PTAG_OWNER).toString();
}

QString ProjectBackend::crDate(const QString& hash)
{
    return ::property(hash, PTAG_CRDATE).toString();
}

QString ProjectBackend::mfDate(const QString& hash)
{
    return ::property(hash, PTAG_MFDATE).toString();
}

QString ProjectBackend::size(const QString& hash)
{
    return ::property(hash, PTAG_SIZE).toString();
}

QString ProjectBackend::hash()
{
    return s_currentHash;
}

QStringList ProjectBackend::projects()
{
    QStringList hashes;
    const auto& udir = UserBackend::instance()->dir();

    if (udir.isEmpty())
        return hashes;

    for (const auto& dir : lsdir(udir)) {
        const auto& p = udir + separator() + dir;
        if (isOwprj(p))
            hashes << ::hash(p);
    }

    return hashes;
}

void ProjectBackend::updateSize()
{
    const auto& dir = ::dir(hash());
    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_SIZE, byteString(dsize(dir)));
}

void ProjectBackend::updateLastModification()
{
    auto date = QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ");
    const auto& dir = ::dir(hash());
    if (dir.isEmpty())
        return;

    ::setProperty(dir, PTAG_MFDATE, date);
}

bool ProjectBackend::start(const QString& hash)
{
    if (ProjectBackend::hash() == hash)
        return true;

    const auto& dir = ::dir(hash);

    if (dir.isEmpty())
        return false;

    if (!ProjectBackend::hash().isEmpty())
        stop();

    s_currentHash = hash;

    emit instance()->started();

    return true;
}

void ProjectBackend::stop()
{
    updateSize();
    updateLastModification();
    s_currentHash = "";
    emit instance()->stopped();
}
