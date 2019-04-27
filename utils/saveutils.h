#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QVariant>

namespace SaveUtils {

enum ProjectProperties {
    ProjectName = 0x1000,
    ProjectDescription,
    ProjectChecksum,
    ProjectCreationDate,
    ProjectModificationDate,
    ProjectSize,
    ProjectUid,
    ProjectTheme,
    ProjectScaling,
    ProjectPropertiesVersion,
    ProjectPropertiesSignature
};

enum ControlProperties {
    ControlId = 0x2000,
    ControlUid,
    ControlChecksum,
    ControlIcon,
    ControlToolName,
    ControlToolCategory,
    ControlPropertiesVersion,
    ControlPropertiesSignature
};

bool isForm(const QString& controlDir);
bool isControlValid(const QString& controlDir);
bool isProjectValid(const QString& projectDir);

QString mainQmlFile();
QString controlMetaFile();
QString projectMetaFile();

QString toMainQmlFile(const QString& controlDir);
QString toControlMetaFile(const QString& controlDir);
QString toThisDir(const QString& controlDir);
QString toChildrenDir(const QString& controlDir);
QString toParentDir(const QString& controlDir);
QString toProjectMetaFile(const QString& projectDir);
QString toDesignsDir(const QString& projectDir);
QString toImportsDir(const QString& projectDir);
QString toOwDir(const QString& projectDir);
QString toGlobalDir(const QString& projectDir);

QString id(const QString& controlDir);
QString uid(const QString& controlDir);
QString name(const QString& controlDir);
QString category(const QString& controlDir);
QByteArray icon(const QString& controlDir);

QString projectUid(const QString& projectDir);
QString projectName(const QString& projectDir);
QString projectSize(const QString& projectDir);
QString projectCreationDate(const QString& projectDir);
QString projectModificationDate(const QString& projectDir);
QString projectScaling(const QString& projectDir);
QString projectDescription(const QString& projectDir);
QJsonValue projectTheme(const QString& projectDir);

QMap<ControlProperties, QVariant> controlMap(const QString& controlDir);
QMap<ProjectProperties, QVariant> projectMap(const QString& projectDir);

QVariant property(const QString& controlDir, ControlProperties property);
QVariant property(const QString& projectDir, ProjectProperties property);

void setProperty(const QString& controlDir, ControlProperties property, const QVariant& value);
void setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value);

void regenerateUids(const QString& topPath);
QStringList formPaths(const QString& projectDir);
QStringList childrenPaths(const QString& controlDir);

} // SaveUtils

QDataStream& operator>>(QDataStream& in, SaveUtils::ProjectProperties& e);
QDataStream& operator<<(QDataStream& out, SaveUtils::ProjectProperties e);
QDataStream& operator>>(QDataStream& in, SaveUtils::ControlProperties& e);
QDataStream& operator<<(QDataStream& out, SaveUtils::ControlProperties e);

#endif // SAVEUTILS_H
