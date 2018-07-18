#include <saveutils.h>
#include <filemanager.h>
#include <hashfactory.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

/*
                   DATABASE INFRASTRUCTURE

    POPERTIES:
        Elements:
            Main-form(master), Ordinary-form(master),
            Child-item(master), Child-item(non-master)

        Types:
            Quick, Non-gui, Window

    RULES:
        Non-gui elements can not be master (or form)
        Main form has to be window type
        Other forms could be quick item or window type (not non-gui)
        Children could be non-gui or quick item type (not window)
        A form has to be master item
*/

namespace {

void exchangeMatchesForFile(const QString& filePath, const QString& from, const QString& to)
{
    QByteArray data = rdfile(filePath);
    data.replace(from.toUtf8(), to.toUtf8());
    wrfile(filePath, data);
}

QJsonObject rootJsonObjectForFile(const QString& fileName)
{
    return QJsonDocument::fromJson(rdfile(fileName)).object();
}

void writeJsonObjectToFile(const QJsonObject& jsonObject, const QString& fileName)
{
    wrfile(fileName, QJsonDocument(jsonObject).toJson());
}
}

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

/*
    Counts all children paths (rootPath) within given root path.
    Returns children count only if they have match between their and given suid.
    If given suid is empty then rootPath's uid is taken.
*/
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

QString SaveUtils::toUrl(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + FILE_MAIN;
}

QString SaveUtils::toIcon(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + FILE_ICON;
}

QString SaveUtils::toThisDir(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS;
}

QString SaveUtils::toParentDir(const QString& topPath)
{
    return dname(dname(topPath));
}

QString SaveUtils::toChildrenDir(const QString& rootPath)
{
    return rootPath + separator() + DIR_CHILDREN;
}

QString SaveUtils::toProjectFile(const QString& projectDir)
{
    return projectDir + separator() + FILE_PROJECT;
}

QString SaveUtils::toControlFile(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + FILE_CONTROL;
}

QStringList SaveUtils::formPaths(const QString& projectDir)
{
    QStringList paths;

    if (projectDir.isEmpty())
        return paths;

    const QString& dirOwdb = projectDir + separator() + DIR_OWDB;
    for (const QString& formFolder : lsdir(dirOwdb)) {
        const QString& formDir = dirOwdb + separator() + formFolder;
        if (isOwctrl(formDir))
            paths.append(formDir);
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

    for (const QString& controlFilePath : fps(FILE_CONTROL, topPath)) {
        const QString& controlDir = toParentDir(controlFilePath);
        if (isOwctrl(controlDir))
            paths.append(controlDir);
    }

    return paths;
}

QStringList SaveUtils::masterPaths(const QString& topPath)
{
    QStringList paths;
    QStringList foundSuids;

    const QStringList& ctrlPaths = controlPaths(topPath);
    for (const QString& controlPath : ctrlPaths) {
        const QString& controlSuid = suid(controlPath);
        if (!controlSuid.isEmpty() && !foundSuids.contains(controlSuid))
            foundSuids.append(controlSuid);
    }

    for (const QString& controlPath : ctrlPaths) {
        if (foundSuids.contains(uid(controlPath)))
            paths.append(controlPath);
    }

    std::sort(paths.begin(), paths.end(), [] (const QString& a, const QString& b)
    { return a.size() > b.size(); });

    if (paths.isEmpty() && isForm(topPath))
        paths.append(topPath);

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

    if (suid.isEmpty())
        suid = uid(rootPath);

    const QString& childrenDir = toChildrenDir(rootPath);
    for (const QString& childFolder : lsdir(childrenDir)) {
        const QString& childDir = childrenDir + separator() + childFolder;
        if (isOwctrl(childDir) && SaveUtils::suid(childDir) == suid) {
            paths.append(childDir);
            paths.append(childrenPaths(childDir, suid));
        }
    }

    return paths;
}

QString SaveUtils::uid(const QString& rootPath)
{
    return property(rootPath, TAG_UID).toString();
}

QString SaveUtils::suid(const QString& rootPath)
{
    return property(rootPath, TAG_SUID).toString();
}

QString SaveUtils::name(const QString& rootPath)
{
    return property(rootPath, TAG_NAME).toString();
}

QString SaveUtils::category(const QString& rootPath)
{
    return property(rootPath, TAG_CATEGORY).toString();
}

QString SaveUtils::projectHash(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_HASH).toString();
}

QString SaveUtils::projectName(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_NAME).toString();
}

QString SaveUtils::projectDescription(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_DESCRIPTION).toString();
}

QString SaveUtils::projectOwner(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_OWNER).toString();
}

QString SaveUtils::projectCrDate(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_CRDATE).toString();
}

QString SaveUtils::projectMfDate(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_MFDATE).toString();
}

QString SaveUtils::projectSize(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_SIZE).toString();
}

QString SaveUtils::projectScaling(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_SCALING).toString();
}

QJsonValue SaveUtils::projectTheme(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_THEME);
}

QJsonValue SaveUtils::property(const QString& rootPath, const QString& property)
{
    return rootJsonObjectForFile(toControlFile(rootPath)).value(property);
}

QJsonValue SaveUtils::projectProperty(const QString& projectDir, const QString& property)
{
    return rootJsonObjectForFile(toProjectFile(projectDir)).value(property);
}

void SaveUtils::setProperty(const QString& rootPath, const QString& property, const QJsonValue& value)
{
    Q_ASSERT(!rootPath.isEmpty() && !property.isEmpty() && isOwctrl(rootPath));

    const QString& controlFile = toControlFile(rootPath);
    QJsonObject rootJsonObject = rootJsonObjectForFile(controlFile);
    rootJsonObject.insert(property, value);
    writeJsonObjectToFile(rootJsonObject, controlFile);
}

void SaveUtils::setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value)
{
    Q_ASSERT(!projectDir.isEmpty() && !property.isEmpty() && isOwprj(projectDir));

    const QString& projectFile = toProjectFile(projectDir);
    QJsonObject rootJsonObject = rootJsonObjectForFile(projectFile);
    rootJsonObject.insert(property, value);
    writeJsonObjectToFile(rootJsonObject, projectFile);
}

/*
    Recalculates all uids belongs to given control and its children (all).
    Suids and everything that are related to given uid also updated in control files.
*/
void SaveUtils::regenerateUids(const QString& topPath)
{
    if (topPath.isEmpty())
        return;

    const QStringList& controlFiles = fps(FILE_CONTROL, topPath);
    for (const QString& controlFile : controlFiles) {
        const QString& controlDir = toParentDir(controlFile);

        if (!isOwctrl(controlDir))
            continue;

        const QString& controlUid = uid(controlDir);
        const QString& newUid = HashFactory::generate();

        for (const QString& controlFile : controlFiles) {
            if (isOwctrl(toParentDir(controlFile)))
                exchangeMatchesForFile(controlFile, controlUid, newUid);
        }
    }
}
