#include <saveutils.h>
#include <filemanager.h>
#include <hashfactory.h>

#include <QDataStream>
#include <QJsonValue>

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

QString toThisDir(const QString& controlDir)
{
    return controlDir + separator() + DIR_THIS;
}

QString toParentDir(const QString& controlDir)
{
    return dname(dname(controlDir));
}

QString toChildrenDir(const QString& controlDir)
{
    return controlDir + separator() + DIR_CHILDREN;
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

QString toControlFile(const QString& controlDir)
{
    return controlDir + separator() + DIR_THIS + separator() + FILE_CONTROL;
}

QString id(const QString& controlDir)
{
    return property(controlDir, ControlId).toString();
}

QString uid(const QString& controlDir)
{
    return property(controlDir, ControlUid).toString();
}

QString name(const QString& controlDir)
{
    return property(controlDir, ControlToolName).toString();
}

QString category(const QString& controlDir)
{
    return property(controlDir, ControlToolCategory).toString();
}

QByteArray icon(const QString& controlDir)
{
    return property(controlDir, ControlIcon).toByteArray();
}

QString projectUid(const QString& projectDir)
{
    return property(projectDir, ProjectUid).toString();
}

QString projectName(const QString& projectDir)
{
    return property(projectDir, ProjectName).toString();
}

QString projectSize(const QString& projectDir)
{
    return property(projectDir, ProjectSize).toString();
}

QString projectCreationDate(const QString& projectDir)
{
    return property(projectDir, ProjectCreationDate).toString();
}

QString projectModificationDate(const QString& projectDir)
{
    return property(projectDir, ProjectModificationDate).toString();
}

QString projectScaling(const QString& projectDir)
{
    return property(projectDir, ProjectScaling).toString();
}

QString projectDescription(const QString& projectDir)
{
    return property(projectDir, ProjectDescription).toString();
}

QJsonValue projectTheme(const QString& projectDir)
{
    return property(projectDir, ProjectTheme).toJsonValue();
}

QMap<ControlProperties, QVariant> controlMap(const QString& controlDir)
{
    QMap<ControlProperties, QVariant> map;
    QDataStream in(rdfile(toControlFile(controlDir)));
    in >> map;
    return map;
}

QMap<ProjectProperties, QVariant> projectMap(const QString& projectDir)
{
    QMap<ProjectProperties, QVariant> map;
    QDataStream in(rdfile(toProjectFile(projectDir)));
    in >> map;
    return map;
}

QVariant property(const QString& controlDir, ControlProperties property)
{
    return controlMap(controlDir).value(property);
}

QVariant property(const QString& projectDir, ProjectProperties property)
{
    return projectMap(projectDir).value(property);
}

void setProperty(const QString& controlDir, ControlProperties property, const QVariant& value)
{
    QMap<ControlProperties, QVariant> map(controlMap(controlDir));
    map.insert(property, value);
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << map;
    wrfile(toControlFile(controlDir), buffer);
}

void setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value)
{
    QMap<ProjectProperties, QVariant> map(projectMap(projectDir));
    map.insert(property, value);
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << map;
    wrfile(toProjectFile(projectDir), buffer);
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
            if (isOwctrl(toParentDir(controlFile))) {
//                QByteArray data = rdfile(controlFile);
//                data.replace(controlUid.toUtf8(), newUid.toUtf8());
//                wrfile(controlFile, data);
            }
        }
    }
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

} // SaveUtils

QDataStream& operator>>(QDataStream& in, SaveUtils::ProjectProperties& e)
{
    return in >> (int&) e;
}

QDataStream& operator<<(QDataStream& out, SaveUtils::ProjectProperties e)
{
    return out << (int) e;
}

QDataStream& operator>>(QDataStream& in, SaveUtils::ControlProperties& e)
{
    return in >> (int&) e;
}

QDataStream& operator<<(QDataStream& out, SaveUtils::ControlProperties e)
{
    return out << (int) e;
}
