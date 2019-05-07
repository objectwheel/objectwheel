#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QVariant>

namespace SaveUtils {

enum ControlProperties : quint32 {
    ControlId = 0x1000,
    ControlUid,
    ControlIcon,
    ControlToolName,
    ControlToolCategory,
    ControlVersion,
    ControlSignature
};

enum ProjectProperties : quint32 {
    ProjectName = 0x2000,
    ProjectDescription,
    ProjectCreationDate,
    ProjectModificationDate,
    ProjectSize,
    ProjectUid,
    ProjectTheme,
    ProjectHdpiScaling,
    ProjectVersion,
    ProjectSignature
};

enum UserProperties : quint32 {
    UserEmail = 0x3000,
    UserFirst,
    UserLast,
    UserCountry,
    UserCompany,
    UserTitle,
    UserPhone,
    UserIcon,
    UserPlan,
    UserRegistrationDate,
    UserLastOnlineDate,
    UserHash,
    UserVersion,
    UserSignature
};

bool isForm(const QString& controlDir);
bool isControlValid(const QString& controlDir);
bool isProjectValid(const QString& projectDir);
bool isUserValid(const QString& userDir);

QString controlMetaFileName();
QString projectMetaFileName();
QString userMetaFileName();
QString mainQmlFileName();

QString toControlMetaFile(const QString& controlDir);
QString toProjectMetaFile(const QString& projectDir);
QString toUserMetaFile(const QString& userDir);
QString toMainQmlFile(const QString& controlDir);
QString toThisDir(const QString& controlDir);
QString toChildrenDir(const QString& controlDir);
QString toParentDir(const QString& controlDir);
QString toDesignsDir(const QString& projectDir);
QString toImportsDir(const QString& projectDir);
QString toOwDir(const QString& projectDir);
QString toGlobalDir(const QString& projectDir);

QString id(const QString& controlDir);
QString uid(const QString& controlDir);
QString name(const QString& controlDir);
QString category(const QString& controlDir);
QByteArray icon(const QString& controlDir);

bool projectHdpiScaling(const QString& projectDir);
qint64 projectSize(const QString& projectDir);
QString projectUid(const QString& projectDir);
QString projectName(const QString& projectDir);
QString projectDescription(const QString& projectDir);
QDateTime projectCreationDate(const QString& projectDir);
QDateTime projectModificationDate(const QString& projectDir);
QJsonValue projectTheme(const QString& projectDir);

quint32 userPlan(const QString& userDir);
QString userEmail(const QString& userDir);
QString userFirst(const QString& userDir);
QString userLast(const QString& userDir);
QString userCountry(const QString& userDir);
QString userCompany(const QString& userDir);
QString userTitle(const QString& userDir);
QString userPhone(const QString& userDir);
QByteArray userHash(const QString& userDir);
QByteArray userIcon(const QString& userDir);
QDateTime userLastOnlineDate(const QString& userDir);
QDateTime userRegistrationDate(const QString& userDir);

QMap<ControlProperties, QVariant> controlMap(const QString& controlDir);
QMap<ProjectProperties, QVariant> projectMap(const QString& projectDir);
QMap<UserProperties, QVariant> userMap(const QString& userDir);

QVariant property(const QString& controlDir, ControlProperties property);
QVariant property(const QString& projectDir, ProjectProperties property);
QVariant property(const QString& userDir, UserProperties property);

void setProperty(const QString& controlDir, ControlProperties property, const QVariant& value);
void setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value);
void setProperty(const QString& userDir, UserProperties property, const QVariant& value);

void makeControlMetaFile(const QString& controlDir);
void makeProjectMetaFile(const QString& projectDir);
void makeUserMetaFile(const QString& userDir);

void regenerateUids(const QString& topPath);
QStringList formPaths(const QString& projectDir);
QStringList childrenPaths(const QString& controlDir);

} // SaveUtils

#endif // SAVEUTILS_H
