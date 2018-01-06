#include <projectbackend.h>
#include <userbackend.h>
#include <savebackend.h>
#include <filemanager.h>
#include <zipper.h>
#include <random>

#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QCryptographicHash>

#define FILENAME    "project.json"
#define PROJECTNAME "projectName"
#define DESCRIPTION "description"
#define OWNER       "owner"
#define CRDATE      "crDate"
#define MFDATE      "mfDate"
#define SIZE        "size"
#define HASH        "hash"
#define OWPRJ       "owprj"
#define SIGN_OWPRJ  "T3dwcmpfdjIuMA"

typedef QCryptographicHash Hasher;
static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_int_distribution<qint32> rand_dist(-2147483647 - 1, 2147483647); //Wow C++

static int biggestDir(const QString& basePath)
{
    int num = 0;
    for (auto dir : lsdir(basePath))
        if (dir.toInt() > num)
            num = dir.toInt();
    return num;
}

static QString byteString(const qint64 size)
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

static void setProperty(
    const QString& rootPath,
    const QString& property,
    const QVariant& value
    )
{
    auto jobj = QJsonDocument::fromJson(
        rdfile(
            rootPath +
            separator() +
            FILENAME
        )
    ).object();

    jobj[property] = value.toJsonValue();
    jobj[MFDATE] = QDateTime::currentDateTime().
                   toString(Qt::ISODate).replace("T", " ");

    const auto& data = QJsonDocument(jobj).toJson();

    wrfile(
        rootPath +
        separator() +
        FILENAME,
        data
    );
}

static bool isOwprj(const QString& rootPath)
{
    const auto& jobj = QJsonDocument::fromJson(
        rdfile(
            rootPath +
            separator() +
            FILENAME
        )
    ).object();

    return (jobj.value(OWPRJ).toString() == SIGN_OWPRJ);
}

static QString hash(const QString& rootPath)
{
    const auto& jobj = QJsonDocument::fromJson(
        rdfile(
            rootPath +
            separator() +
            FILENAME
        )
    ).object();

    return jobj.value(HASH).toString();
}

static QString newHash()
{
    QByteArray data;
    auto randNum = rand_dist(mt);
    auto randNum1 = rand_dist(mt);
    auto randNum2 = rand_dist(mt);
    auto dateMs = QDateTime::currentMSecsSinceEpoch();
    data.insert(0, QString::number(dateMs));
    data.insert(0, QString::number(randNum));
    data.insert(0, QString::number(randNum1));
    data.insert(0, QString::number(randNum2));
    return Hasher::hash(data, Hasher::Md5).toHex();
}

static QString dir(const QString& hash)
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

static QVariant property(const QString& hash, const QString& property)
{
    const auto& dir = ::dir(hash);

    if (dir.isEmpty())
        return QVariant();

    const auto& jobj = QJsonDocument::fromJson(
        rdfile(
            dir +
            separator() +
            FILENAME
        )
    ).object();

    return jobj.value(property).toVariant();
}

ProjectBackend* ProjectBackend::instance()
{
    static ProjectBackend instance;
    return &instance;
}

bool ProjectBackend::newProject(
    const QString& name,
    const QString& description,
    const QString& owner,
    const QString& crDate,
    const QString& size
  ) const
{
    const auto& udir = UserBackend::instance()->dir();

    if (udir.isEmpty() ||
        name.isEmpty() ||
        description.isEmpty() ||
        owner.isEmpty() ||
        crDate.isEmpty() ||
        size.isEmpty())
        return false;

    const auto& pdir = udir + separator() +
       QString::number(biggestDir(udir) + 1);

    QJsonObject jobj;
    jobj[PROJECTNAME] = name;
    jobj[DESCRIPTION] = description;
    jobj[OWNER]       = owner;
    jobj[CRDATE]      = crDate;
    jobj[MFDATE]      = crDate;
    jobj[SIZE]        = size;
    jobj[OWPRJ]       = SIGN_OWPRJ;
    jobj[HASH]        = newHash();

    const auto& data = QJsonDocument(jobj).toJson();

    return(
        !mkdir(pdir) ||
        0 > wrfile(
            pdir + separator() +
            FILENAME, data
        ) ||
        SaveBackend::initProject(pdir)
                );
}

QStringList ProjectBackend::projectNames() const
{
    QStringList names;
    const auto& hashes = projects();

    if (hashes.isEmpty())
        return names;

    for (const auto& hash : hashes)
        names << ::property(hash, PROJECTNAME).toString();

    return names;
}

void ProjectBackend::changeName(const QString& hash, const QString& name) const
{
    const auto& dir = ::dir(hash);

    if (dir.isEmpty())
        return;

    ::setProperty(dir, PROJECTNAME, name);
}

void ProjectBackend::changeDescription(const QString& hash, const QString& desc) const
{
    const auto& dir = ::dir(hash);

    if (dir.isEmpty())
        return;

    ::setProperty(dir, DESCRIPTION, desc);
}

bool ProjectBackend::exportProject(const QString& hash, const QString& filePath) const
{
    const auto& dir = ::dir(hash);

    if (dir.isEmpty() || filePath.isEmpty())
        return false;

    return Zipper::compressDir(dir, filePath);
}

bool ProjectBackend::importProject(const QString &filePath) const
{
    const auto& data = rdfile(filePath);
    const auto& udir = UserBackend::instance()->dir();
    const auto& pdir = udir + separator() +
      QString::number(biggestDir(udir) + 1);

    if (data.isEmpty() || udir.isEmpty())
        return false;

    if (!mkdir(pdir) || !Zipper::extractZip(data, pdir))
        return false;

    ::setProperty(pdir, HASH, newHash());

    return true;
}

QString ProjectBackend::dir(const QString& hash) const
{
    return ::dir(hash);
}

QString ProjectBackend::name(const QString& hash) const
{
    return ::property(hash, PROJECTNAME).toString();
}

QString ProjectBackend::description(const QString& hash) const
{
    return ::property(hash, DESCRIPTION).toString();
}

QString ProjectBackend::owner(const QString& hash) const
{
    return ::property(hash, OWNER).toString();
}

QString ProjectBackend::crDate(const QString& hash) const
{
    return ::property(hash, CRDATE).toString();
}

QString ProjectBackend::mfDate(const QString& hash) const
{
    return ::property(hash, MFDATE).toString();
}

QString ProjectBackend::size(const QString& hash) const
{
    return ::property(hash, SIZE).toString();
}

const QString& ProjectBackend::hash() const
{
    return _currentHash;
}

QStringList ProjectBackend::projects() const
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

void ProjectBackend::updateSize() const
{
    const auto& dir = ::dir(hash());

    if (dir.isEmpty())
        return;

    ::setProperty(dir, SIZE, byteString(dsize(dir)));
}

void ProjectBackend::updateLastModification() const
{
    auto date =
    QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ");

    const auto& dir = ::dir(hash());

    if (dir.isEmpty())
        return;

    ::setProperty(dir, MFDATE, date);
}

bool ProjectBackend::start(const QString& hash)
{
    if (this->hash() == hash)
		return true;

    const auto& dir = ::dir(hash);

    if (dir.isEmpty())
        return false;

    if (!this->hash().isEmpty())
        stop();

    _currentHash = hash;

    emit started();

	return true;
}

void ProjectBackend::stop()
{
    updateSize();
    updateLastModification();
    _currentHash = "";
    emit stopped();
}
