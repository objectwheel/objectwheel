#include <saveutils.h>
#include <filemanager.h>
#include <hashfactory.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/******************************************************************/
/**          D A T A B A S E  I N F R A S T R U C T U R E        **/
/******************************************************************/
/** POPERTIES:                                                   **/
/*  Elements: Main-form(master), Ordinary-form(master),           */
/*            Child-item(master), Child-item(non-master)          */
/*  Types:    Quick, Non-gui, Window                              */
/*                                                                */
/** RULES:                                                       **/
/* - Non-gui elements can not be master (or form)                 */
/* - Main form has to be window type                              */
/* - Other forms could be quick item or window type (not non-gui) */
/* - Children could be non-gui or quick item type (not window)    */
/* - A form has to be master item                                 */
/******************************************************************/

bool SaveUtils::isForm(const QString& rootPath)
{
    return DIR_OWDB == fname(dname(rootPath));
}

/*
    Returns true if given path belongs to main form
    It doesn't check whether rootPath belong to a form or not.
*/
bool SaveUtils::isMain(const QString& rootPath)
{
    return fname(rootPath) == DIR_MAINFORM;
}

bool SaveUtils::isOwctrl(const QString& rootPath)
{
    const QString& sign = property(rootPath, TAG_OWCTRL_SIGN).toString();
    return sign == SIGN_OWCTRL && !uid(rootPath).isEmpty();
}

bool SaveUtils::isOwprj(const QString& projectDir)
{
    const QString& sign = projectProperty(projectDir, PTAG_OWPRJ_SIGN).toString();
    return sign == SIGN_OWPRJ;
}

/*
    Returns biggest number from integer named dirs.
    If no integer named dir exists, 0 returned.
    If no dir exists or dirs are smaller than zero, 0 returned.
*/
int SaveUtils::biggestDir(const QString& basePath)
{
    int num = 0;
    for (const QString& dir : lsdir(basePath)) {
        if (dir.toInt() > num)
            num = dir.toInt();
    }
    return num;
}

int SaveUtils::childrenCount(const QString& rootPath, QString suid)
{
    int counter = 0;

    if (rootPath.isEmpty())
        return counter;

    if (suid.isEmpty())
        suid = uid(rootPath);

    const QString& childrenDir = toChildrenDir(rootPath);
    for (const QString& childFolder : lsdir(childrenDir)) {
        const QString& childDir = childrenDir + separator() + childFolder;
        if (isOwctrl(childDir) && SaveUtils::suid(childDir) == suid) {
            ++counter;
            counter += childrenCount(childDir, suid);
        }
    }

    return counter;
}

QStringList SaveUtils::formPaths(const QString& projectDir)
{
    QStringList paths;

    if (projectDir.isEmpty())
        return paths;

    const QString& baseDir = projectDir + separator() + DIR_OWDB;

    for (const QString& dir : lsdir(baseDir)) {
        const QString& propertyPath = toPropertiesFile(baseDir + separator() + dir);
        if (isOwctrl(propertyPath))
            paths << toParentDir(propertyPath);
    }

    return paths;
}

/*
    Searches for all controls paths, starting from topPath.
    Returns all control paths (rootPaths) within given topPath.
*/
QStringList SaveUtils::controlPaths(const QString& topPath)
{
    QStringList paths;

    if (topPath.isEmpty())
        return paths;

    for (const QString& path : fps(FILE_PROPERTIES, topPath)) {
        if (isOwctrl(path))
            paths << toParentDir(path);
    }

    return paths;
}

QStringList SaveUtils::masterPaths(const QString& topPath)
{
    QStringList paths;
    QStringList foundSuids;

    const QStringList& ctrlPaths = controlPaths(topPath);

    for (const QString& path : ctrlPaths) {
        const QString& controlSuid = suid(path);

        if (!controlSuid.isEmpty() && !foundSuids.contains(controlSuid))
            foundSuids << controlSuid;
    }

    for (const QString& path : ctrlPaths) {
        if (foundSuids.contains(uid(path)))
            paths << path;
    }

    std::sort(paths.begin(), paths.end(), []( const QString& a, const QString& b)
    { return a.size() > b.size(); });

    if (paths.isEmpty() && isForm(topPath))
        paths << topPath;

    return paths;
}

/*
    Returns all children paths (rootPath) within given root path.
    Returns children only if they have match between their and given suid.
    If given suid is empty then rootPath's uid is taken.
*/
QStringList SaveUtils::childrenPaths(const QString& rootPath, QString suid)
{
    QStringList paths;

    if (rootPath.isEmpty())
        return paths;

    if (suid.isEmpty()) {
        const QString& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
        suid = property(propertyPath, TAG_UID).toString();
    }

    const QString& childrenPath = rootPath + separator() + DIR_CHILDREN;
    for (const QString& dir : lsdir(childrenPath)) {
        const QString& propertyPath = childrenPath + separator() + dir +
                separator() + DIR_THIS + separator() + FILE_PROPERTIES;

        if (isOwctrl(propertyPath) && property(propertyPath, TAG_SUID).toString() == suid) {
            paths << toParentDir(propertyPath);
            paths << childrenPaths(toParentDir(propertyPath), suid);
        }
    }

    return paths;
}

qreal SaveUtils::x(const QString& rootPath)
{
    const QString& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    return property(propertyPath, TAG_X).toDouble();
}

qreal SaveUtils::y(const QString& rootPath)
{
    const QString& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    return property(propertyPath, TAG_Y).toDouble();
}

qreal SaveUtils::z(const QString& rootPath)
{
    const QString& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    return property(propertyPath, TAG_Z).toDouble();
}

qreal SaveUtils::width(const QString& rootPath)
{
    const QString& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    return property(propertyPath, TAG_WIDTH).toDouble();
}

qreal SaveUtils::height(const QString& rootPath)
{
    const QString& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    return property(propertyPath, TAG_HEIGHT).toDouble();
}









void SaveUtils::setX(const QString& rootPath, qreal x)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_X, x);
    wrfile(propertyPath, propertyData);
}

void SaveUtils::setY(const QString& rootPath, qreal y)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_Y, y);
    wrfile(propertyPath, propertyData);
}

void SaveUtils::setZ(const QString& rootPath, qreal z)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_Z, z);
    wrfile(propertyPath, propertyData);
}

void SaveUtils::setWidth(const QString& rootPath, qreal width)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_WIDTH, width);
    wrfile(propertyPath, propertyData);
}

void SaveUtils::setHeight(const QString& rootPath, qreal height)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_HEIGHT, height);
    wrfile(propertyPath, propertyData);
}

void SaveUtils::setId(const QString& rootPath, const QString& id)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_ID, id);
    wrfile(propertyPath, propertyData);
}

// Update all matching 'from's to 'to's within given file
void SaveUtils::updateFile(const QString& filePath, const QString& from, const QString& to)
{
    auto data = rdfile(filePath);
    data.replace(from.toUtf8(), to.toUtf8());
    wrfile(filePath, data);
}

// Recalculates all uids belongs to given control and its children (all).
// Both database and in-memory data are updated.
void SaveUtils::recalculateUids(const QString& topPath)
{
    if (topPath.isEmpty())
        return;

    QStringList paths, properties;

    properties << fps(FILE_PROPERTIES, topPath);
    paths << properties;

    for (auto pfile : properties) {
        auto propertyData = rdfile(pfile);

        if (!SaveUtils::isOwctrl(propertyData))
            continue;

        auto uid = SaveUtils::property(propertyData, TAG_UID).toString();
        auto newUid = HashFactory::generate();

        for (auto file : paths)
            SaveUtils::updateFile(file, uid, newUid);
    }
}

QString SaveUtils::id(const QString& rootPath)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, TAG_ID).toString();
}

QString SaveUtils::uid(const QString& rootPath)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, TAG_UID).toString();
}

QString SaveUtils::suid(const QString& rootPath)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, TAG_SUID).toString();
}

QString SaveUtils::toUrl(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + "main.qml";
}

QString SaveUtils::toProjectFile(const QString& projectDir)
{
    return projectDir + separator() + FILE_PROJECT;
}

QString SaveUtils::toPropertiesFile(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
}

QString SaveUtils::toParentDir(const QString& topPath)
{
    return dname(dname(topPath));
}

QString SaveUtils::toChildrenDir(const QString& rootPath)
{
    return rootPath + separator() + DIR_CHILDREN;
}

QString SaveUtils::toolName(const QString& toolRootPath)
{
    const auto& propertyPath = toolRootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, TAG_NAME).toString();
}

QString SaveUtils::toolCategory(const QString& toolRootPath)
{
    const auto& propertyPath = toolRootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, TAG_CATEGORY).toString();
}

QString SaveUtils::hash(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_HASH).toString();
}

QString SaveUtils::projectName(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_PROJECTNAME).toString();
}

QString SaveUtils::description(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_DESCRIPTION).toString();
}

QString SaveUtils::owner(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_OWNER).toString();
}

QString SaveUtils::crDate(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_CRDATE).toString();
}

QString SaveUtils::mfDate(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_MFDATE).toString();
}

QString SaveUtils::size(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_SIZE).toString();
}

QString SaveUtils::scaling(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_SCALING).toString();
}

QJsonValue SaveUtils::theme(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_THEME);
}

void SaveUtils::setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value)
{
    Q_ASSERT(!projectDir.isEmpty() && isOwprj(projectDir));

    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, property, value);
    wrfile(propertyPath, propertyData);
}
