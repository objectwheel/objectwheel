#include <saveutils.h>
#include <filemanager.h>
#include <hashfactory.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

// TODO: Change QString with QLatin1String whenever it is possible
// TODO: Always use case insensitive comparison when it is possible

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

namespace SaveUtils {

bool isForm(const QString& rootPath)
{
    return DIR_OWDB == fname(dname(rootPath));
}

/*!
    Returns true if given path belongs to main form
    It doesn't check whether rootPath belong to a form or not.
*/
bool isMain(const QString& rootPath)
{
    return fname(rootPath) == DIR_MAINFORM;
}

bool isOwctrl(const QString& rootPath)
{
    Q_ASSERT(!rootPath.isEmpty());
    const QString& sign = property(rootPath, TAG_OWCTRL_SIGN).toString();
    return sign == SIGN_OWCTRL && !uid(rootPath).isEmpty();
}

bool isOwprj(const QString& projectDir)
{
    const QString& sign = projectProperty(projectDir, PTAG_OWPRJ_SIGN).toString();
    return sign == SIGN_OWPRJ;
}

/*!
    Returns biggest number from integer named dirs.
    If no integer named dir exists, 0 returned.
    If no dir exists or dirs are smaller than zero, 0 returned.
*/
int biggestDir(const QString& basePath)
{
    int num = 0;
    for (const QString& dir : lsdir(basePath)) {
        if (dir.toInt() > num)
            num = dir.toInt();
    }
    return num;
}

/*!
    Counts all children paths (rootPath) within given root path.
*/
int childrenCount(const QString& rootPath)
{
    Q_ASSERT(!rootPath.isEmpty());

    int counter = 0;
    const QString& childrenDir = toChildrenDir(rootPath);
    for (const QString& childDirName : lsdir(childrenDir)) {
        const QString& childRootPath = childrenDir + separator() + childDirName;
        if (isOwctrl(childRootPath)) {
            ++counter;
            counter += childrenCount(childRootPath);
        }
    }

    return counter;
}

QString toUrl(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + FILE_MAIN;
}

QString toIcon(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + FILE_ICON;
}

QString toThisDir(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS;
}

QString toParentDir(const QString& topPath)
{
    return dname(dname(topPath));
}

QString toChildrenDir(const QString& rootPath)
{
    return rootPath + separator() + DIR_CHILDREN;
}

QString toOwdbDir(const QString& projectDir)
{
    return projectDir + separator() + DIR_OWDB;
}

QString toProjectFile(const QString& projectDir)
{
    return projectDir + separator() + FILE_PROJECT;
}

QString toImportsDir(const QString& projectDir)
{
    return toOwdbDir(projectDir) + separator() + DIR_IMPORTS;
}

QString toOwDir(const QString& projectDir)
{
    return toImportsDir(projectDir) + separator() + DIR_OW;
}

QString toGlobalDir(const QString& projectDir)
{
    return toOwDir(projectDir) + separator() + DIR_GLOBAL;
}

QString toControlFile(const QString& rootPath)
{
    return rootPath + separator() + DIR_THIS + separator() + FILE_CONTROL;
}

QStringList formPaths(const QString& projectDir)
{
    QStringList paths;

    if (projectDir.isEmpty())
        return paths;

    const QString& dirOwdb = toOwdbDir(projectDir);
    for (const QString& formFolder : lsdir(dirOwdb)) {
        const QString& formDir = dirOwdb + separator() + formFolder;
        if (isOwctrl(formDir))
            paths.append(formDir);
    }

    return paths;
}

/*!
    Searches for all controls paths, starting from topPath.
    Returns all control paths (rootPaths) within given topPath.
*/
QStringList controlPaths(const QString& topPath)
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

/*!
    Returns all children paths (rootPath) within given root path.
*/
QStringList childrenPaths(const QString& rootPath)
{
    Q_ASSERT(!rootPath.isEmpty());

    QStringList paths;
    const QString& childrenDir = toChildrenDir(rootPath);
    for (const QString& childDirName : lsdir(childrenDir)) {
        const QString& childRootPath = childrenDir + separator() + childDirName;
        if (isOwctrl(childRootPath)) {
            paths.append(childRootPath);
            paths.append(childrenPaths(childRootPath));
        }
    }

    return paths;
}

QString id(const QString& rootPath)
{
    return property(rootPath, TAG_ID).toString();
}

QString uid(const QString& rootPath)
{
    return property(rootPath, TAG_UID).toString();
}

QString name(const QString& rootPath)
{
    return property(rootPath, TAG_NAME).toString();
}

QString category(const QString& rootPath)
{
    return property(rootPath, TAG_CATEGORY).toString();
}

QString projectUid(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_UID).toString();
}

QString projectName(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_NAME).toString();
}

QString projectDescription(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_DESCRIPTION).toString();
}

QString projectOwner(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_OWNER).toString();
}

QString projectCrDate(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_CRDATE).toString();
}

QString projectMfDate(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_MFDATE).toString();
}

QString projectSize(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_SIZE).toString();
}

QString projectScaling(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_SCALING).toString();
}

QJsonValue projectTheme(const QString& projectDir)
{
    return projectProperty(projectDir, PTAG_THEME);
}

QJsonValue property(const QString& rootPath, const QString& property)
{
    return rootJsonObjectForFile(toControlFile(rootPath)).value(property);
}

QJsonValue projectProperty(const QString& projectDir, const QString& property)
{
    return rootJsonObjectForFile(toProjectFile(projectDir)).value(property);
}

void setProperty(const QString& rootPath, const QString& property, const QJsonValue& value)
{
    Q_ASSERT(!rootPath.isEmpty() && !property.isEmpty() && isOwctrl(rootPath));
    const QString& controlFile = toControlFile(rootPath);
    QJsonObject rootJsonObject = rootJsonObjectForFile(controlFile);
    rootJsonObject.insert(property, value);
    writeJsonObjectToFile(rootJsonObject, controlFile);
}

void setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value)
{
    Q_ASSERT(!projectDir.isEmpty() && !property.isEmpty() && isOwprj(projectDir));
    const QString& projectFile = toProjectFile(projectDir);
    QJsonObject rootJsonObject = rootJsonObjectForFile(projectFile);
    rootJsonObject.insert(property, value);
    writeJsonObjectToFile(rootJsonObject, projectFile);
}

/*!
    Recalculates all uids belongs to given control and its children (all).
*/
void regenerateUids(const QString& topPath)
{
    Q_ASSERT(!topPath.isEmpty());
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
} // SaveUtils