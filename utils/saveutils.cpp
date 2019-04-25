#include <saveutils.h>
#include <filemanager.h>
#include <hashfactory.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

// TODO: Always use case insensitive comparison when it is possible

#define SIGN_OWCTRL      "T3djdHJsX3YyLjA"
#define SIGN_OWPRJT      "T3dwcmpfdjIuMA"

#define DIR_THIS         "t"
#define DIR_CHILDREN     "c"
#define DIR_DESIGNS      "designs"
#define DIR_IMPORTS      "imports"
#define DIR_OW           "Objectwheel"
#define DIR_GLOBAL       "GlobalResources"

#define FILE_PROJECT     "project.meta"
#define FILE_CONTROL     "control.meta"
#define FILE_MAIN        "main.qml"        // TODO: Apply everywhere

namespace SaveUtils {

namespace Internal {

void exchangeMatchesForFile(const QString& filePath, const QString& from, const QString& to)
{
    QByteArray data = rdfile(filePath);
    data.replace(from.toUtf8(), to.toUtf8());
    wrfile(filePath, data);
}

QJsonObject rootJsonObjectForFile(const QString& fileName)
{
    return QJsonDocument::fromBinaryData(rdfile(fileName)).object();
}

void writeJsonObjectToFile(const QJsonObject& jsonObject, const QString& fileName)
{
    wrfile(fileName, QJsonDocument(jsonObject).toBinaryData());
}

} // Internal

bool isForm(const QString& controlDir)
{
    return DIR_DESIGNS == fname(dname(controlDir));
}

bool isOwctrl(const QString& controlDir)
{
    Q_ASSERT(!controlDir.isEmpty());
    const QString& sign = property(controlDir, ControlPropertiesSignature).toString();
    return sign == SIGN_OWCTRL && !uid(controlDir).isEmpty();
}

bool isOwprjt(const QString& projectDir)
{
    const QString& sign = property(projectDir, ProjectPropertiesSignature).toString();
    return sign == SIGN_OWPRJT;
}

QString toMain(const QString& controlDir)
{
    return controlDir + separator() + DIR_THIS + separator() + FILE_MAIN;
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

QString toDesignsDir(const QString& projectDir)
{
    return projectDir + separator() + DIR_DESIGNS;
}

QString toProjectFile(const QString& projectDir)
{
    return projectDir + separator() + FILE_PROJECT;
}

QString toImportsDir(const QString& projectDir)
{
    return projectDir + separator() + DIR_IMPORTS;
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

    const QString& dirDesigns = toDesignsDir(projectDir);
    for (const QString& formFolder : lsdir(dirDesigns)) {
        const QString& formDir = dirDesigns + separator() + formFolder;
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

QByteArray icon(const QString& rootPath)
{
    return QByteArray::fromBase64(property(rootPath, TAG_ICON).toString().toUtf8());
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
    return Internal::rootJsonObjectForFile(toControlFile(rootPath)).value(property);
}

QJsonValue projectProperty(const QString& projectDir, const QString& property)
{
    return Internal::rootJsonObjectForFile(toProjectFile(projectDir)).value(property);
}

void setProperty(const QString& rootPath, const QString& property, const QJsonValue& value)
{
    Q_ASSERT(!rootPath.isEmpty() && !property.isEmpty() && isOwctrl(rootPath));
    const QString& controlFile = toControlFile(rootPath);
    QJsonObject rootJsonObject = Internal::rootJsonObjectForFile(controlFile);
    rootJsonObject.insert(property, value);
    Internal::writeJsonObjectToFile(rootJsonObject, controlFile);
}

void setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value)
{
    Q_ASSERT(!projectDir.isEmpty() && !property.isEmpty() && isOwprjt(projectDir));
    const QString& projectFile = toProjectFile(projectDir);
    QJsonObject rootJsonObject = Internal::rootJsonObjectForFile(projectFile);
    rootJsonObject.insert(property, value);
    Internal::writeJsonObjectToFile(rootJsonObject, projectFile);
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
                Internal::exchangeMatchesForFile(controlFile, controlUid, newUid);
        }
    }
}

} // SaveUtils