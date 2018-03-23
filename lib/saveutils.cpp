#include <saveutils.h>
#include <filemanager.h>
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
/*   unless main form has a Phone skin, in this case other forms  */
/*   (except main form) has to be item (not window or non-gui)    */
/* - Children could be non-gui or quick item type (not window)    */
/* - A form has to be master item                                 */
/******************************************************************/

void SaveUtils::setProperty(QByteArray& propertyData, const QString& property, const QJsonValue& value)
{
    if (propertyData.isEmpty())
        return;

    auto jobj = QJsonDocument::fromJson(propertyData).object();
    jobj[property] = value;
    propertyData = QJsonDocument(jobj).toJson();
}

QJsonValue SaveUtils::property(const QByteArray& propertyData, const QString& property)
{
    if (propertyData.isEmpty())
        return QJsonValue();

    const auto& jobj = QJsonDocument::fromJson(propertyData).object();
    return jobj.value(property);
}

void SaveUtils::flushId(const QString& topPath, const QString& id)
{
    const auto& propertyPath = topPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_ID, id);
    wrfile(propertyPath, propertyData);
}

void SaveUtils::flushSuid(const QString& topPath, const QString& suid)
{
    const auto& fromUid = SaveUtils::suid(topPath);
    if (!fromUid.isEmpty()) {
        for (const auto& path : fps(FILE_PROPERTIES, topPath)) {
            if (SaveUtils::suid(dname(dname(path))) == fromUid) {
                auto propertyData = rdfile(path);
                setProperty(propertyData, TAG_SUID, suid);
                wrfile(path, propertyData);
            }
        }
    } else {
        const auto& propertyPath = topPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        setProperty(propertyData, TAG_SUID, suid);
        wrfile(propertyPath, propertyData);
    }
}

bool SaveUtils::isOwctrl(const QByteArray& propertyData)
{
    const auto& sign = property(propertyData, TAG_OWCTRL_SIGN).toString();
    const auto& uid = property(propertyData, TAG_UID).toString();
    return (sign == SIGN_OWCTRL && !uid.isEmpty());
}

bool SaveUtils::isOwctrl(const QString& rootPath)
{
    const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
    const auto& propertyData = rdfile(propertyPath);
    return isOwctrl(propertyData);
}

bool SaveUtils::isOwprj(const QByteArray& propertyData)
{
    const auto& sign = property(propertyData, PTAG_OWPRJ_SIGN).toString();
    return sign == SIGN_OWPRJ;
}

bool SaveUtils::isOwprj(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return isOwprj(propertyData);
}

// Searches for all controls paths, starting from topPath.
// Returns all control paths (rootPaths) within given topPath.
QStringList SaveUtils::controlPaths(const QString& topPath)
{
    QStringList paths;

    if (topPath.isEmpty())
        return paths;

    for (const auto& path : fps(FILE_PROPERTIES, topPath)) {
        const auto& propertyData = rdfile(path);
        if (isOwctrl(propertyData))
            paths << dname(dname(path));
    }

    return paths;
}

// Update all matching 'from's to 'to's within given file
void SaveUtils::updateFile(const QString& filePath, const QString& from, const QString& to)
{
    auto data = rdfile(filePath);
    data.replace(from.toUtf8(), to.toUtf8());
    wrfile(filePath, data);
}

// Returns biggest number from integer named dirs.
// If no integer named dir exists, 0 returned.
// If no dir exists or dirs are smaller than zero, 0 returned.
int SaveUtils::biggestDir(const QString& basePath)
{
    int num = 0;
    for (const auto& dir : lsdir(basePath))
        if (dir.toInt() > num)
            num = dir.toInt();
    return num;
}

QStringList SaveUtils::formPaths(const QString& projectDir)
{
    QStringList paths;
    if (projectDir.isEmpty())
        return paths;

    auto baseDir = projectDir + separator() + DIR_OWDB;

    for (auto dir : lsdir(baseDir)) {
        auto propertyPath = baseDir + separator() + dir + separator() +
                            DIR_THIS + separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);

        if (isOwctrl(propertyData))
            paths << dname(dname(propertyPath));
    }

    return paths;
}

// Returns all children paths (rootPath) within given root path.
// Returns children only if they have match between their and given suid.
// If given suid is empty then rootPath's uid is taken.
QStringList SaveUtils::childrenPaths(const QString& rootPath, QString suid)
{
    QStringList paths;

    if (rootPath.isEmpty())
        return paths;

    if (suid.isEmpty()) {
        const auto& propertyPath = rootPath + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
        const auto& propertyData = rdfile(propertyPath);
        suid = property(propertyData, TAG_UID).toString();
    }

    const auto& childrenPath = rootPath + separator() + DIR_CHILDREN;
    for (const auto& dir : lsdir(childrenPath)) {
        const auto& propertyPath = childrenPath + separator() + dir + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
        const auto& propertyData = rdfile(propertyPath);

        if (isOwctrl(propertyData) && property(propertyData, TAG_SUID).toString() == suid) {
            paths << dname(dname(propertyPath));
            paths << childrenPaths(dname(dname(propertyPath)), suid);
        }
    }

    return paths;
}

QStringList SaveUtils::masterPaths(const QString& topPath)
{
    QStringList paths;
    const auto& ctrlPaths = controlPaths(topPath);

    QStringList foundSuids;
    for (const auto& path : ctrlPaths) {
        const auto& _suid = suid(path);
        if (!_suid.isEmpty() && !foundSuids.contains(_suid))
            foundSuids << _suid;
    }

    for (const auto& path : ctrlPaths) {
        if (foundSuids.contains(uid(path)))
            paths << path;
    }

    std::sort(paths.begin(), paths.end(),
    [](const QString& a, const QString& b)
    { return a.size() > b.size(); });

    if (paths.isEmpty() && isForm(topPath))
        paths << topPath;

    return paths;
}

bool SaveUtils::isForm(const QString& rootPath)
{
    return DIR_OWDB == fname(dname(rootPath));
}

// Returns true if given path belongs to main form
// It doesn't check whether rootPath belong to a form or not.
bool SaveUtils::isMain(const QString& rootPath)
{
    return (fname(rootPath) == DIR_MAINFORM);
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

QString SaveUtils::theme(const QString& projectDir)
{
    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    const auto& propertyData = rdfile(propertyPath);
    return property(propertyData, PTAG_THEME).toString();
}

void SaveUtils::setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value)
{
    if (projectDir.isEmpty() || !isOwprj(projectDir))
        return;

    const auto& propertyPath = projectDir + separator() + FILE_PROJECT;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, property, value);
    wrfile(propertyPath, propertyData);
}
