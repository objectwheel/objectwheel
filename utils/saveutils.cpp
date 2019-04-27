#include <saveutils.h>
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

namespace SaveUtils {

bool isForm(const QString& controlDir)
{
    return DIR_DESIGNS == fname(dname(controlDir));
}

bool isControlValid(const QString& controlDir)
{
    const QString& sign = property(controlDir, ControlPropertiesSignature).toString();
    return sign == SIGN_OWCTRL && !uid(controlDir).isEmpty();
}

bool isProjectValid(const QString& projectDir)
{
    const QString& sign = property(projectDir, ProjectPropertiesSignature).toString();
    return sign == SIGN_OWPRJT;
}

QString mainQmlFile()
{
    static const QString& mainQmlFile = QStringLiteral("main.qml");
    return mainQmlFile;
}

QString controlMetaFile()
{
    static const QString& controlMetaFile = QStringLiteral("control.meta");
    return controlMetaFile;
}

QString projectMetaFile()
{
    static const QString& projectMetaFile = QStringLiteral("project.meta");
    return projectMetaFile;
}

QString toMainQmlFile(const QString& controlDir)
{
    return controlDir + separator() + DIR_THIS + separator() + mainQmlFile();
}

QString toControlMetaFile(const QString& controlDir)
{
    return controlDir + separator() + DIR_THIS + separator() + FILE_CONTROL;
}

QString toThisDir(const QString& controlDir)
{
    return controlDir + separator() + DIR_THIS;
}

QString toChildrenDir(const QString& controlDir)
{
    return controlDir + separator() + DIR_CHILDREN;
}

QString toParentDir(const QString& controlDir)
{
    return dname(dname(controlDir));
}

QString toProjectMetaFile(const QString& projectDir)
{
    return projectDir + separator() + FILE_PROJECT;
}

QString toDesignsDir(const QString& projectDir)
{
    return projectDir + separator() + DIR_DESIGNS;
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
    QDataStream in(rdfile(toControlMetaFile(controlDir)));
    in >> map;
    return map;
}

QMap<ProjectProperties, QVariant> projectMap(const QString& projectDir)
{
    QMap<ProjectProperties, QVariant> map;
    QDataStream in(rdfile(toProjectMetaFile(projectDir)));
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
    wrfile(toControlMetaFile(controlDir), buffer);
}

void setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value)
{
    QMap<ProjectProperties, QVariant> map(projectMap(projectDir));
    map.insert(property, value);
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << map;
    wrfile(toProjectMetaFile(projectDir), buffer);
}

void regenerateUids(const QString& topPath)
{
    for (const QString& controlFilePath : fps(FILE_CONTROL, topPath)) {
        const QString& controlDir = toParentDir(controlFilePath);
        if (!isControlValid(controlDir))
            continue;
        setProperty(controlDir, ControlUid, HashFactory::generate());
    }
}

QStringList formPaths(const QString& projectDir)
{
    QStringList paths;
    const QString& designsDir = toDesignsDir(projectDir);
    for (const QString& formDirName : lsdir(designsDir)) {
        const QString& formDir = designsDir + separator() + formDirName;
        if (isControlValid(formDir))
            paths.append(formDir);
    }
    return paths;
}

QStringList childrenPaths(const QString& controlDir)
{
    QStringList paths;
    const QString& childrenDir = toChildrenDir(controlDir);
    for (const QString& childDirName : lsdir(childrenDir)) {
        const QString& childControlDir = childrenDir + separator() + childDirName;
        if (isControlValid(childControlDir)) {
            paths.append(childControlDir);
            paths.append(childrenPaths(childControlDir));
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
