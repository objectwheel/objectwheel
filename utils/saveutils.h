#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QVariant>

namespace SaveUtils {

enum ControlProperties : quint32 {
    ControlSignature = 0x1000,
    ControlVersion,
    ControlId,
    ControlUid,
    ControlIcon,
    ControlToolName,
    ControlToolCategory
};

enum ProjectProperties : quint32 {
    ProjectSignature = 0x2000,
    ProjectVersion,
    ProjectUid,
    ProjectName,
    ProjectDescription,
    ProjectCreationDate,
    ProjectModificationDate,
    ProjectSize,
    ProjectTheme,
    ProjectHdpiScaling
};

enum UserProperties : quint32 {
    UserSignature = 0x3000,
    UserVersion,
    UserEmail,
    UserPassword,
    UserFirst,
    UserLast,
    UserCountry,
    UserCompany,
    UserTitle,
    UserPhone,
    UserIcon,
    UserPlan,
    UserRegistrationDate,
    UserLastOnlineDate
};

using ControlMetaHash = QHash<ControlProperties, QVariant>;
using ProjectMetaHash = QHash<ProjectProperties, QVariant>;
using UserMetaHash = QHash<UserProperties, QVariant>;

QString controlMainQmlFileName();
QString controlIconFileName();
QString controlMetaFileName();
QString projectMetaFileName();
QString userIconFileName();
QString userMetaFileName();

QString toControlThisDir(const QString& controlDir);
QString toControlChildrenDir(const QString& controlDir);
QString toControlMetaDir(const QString& controlDir);
QString toProjectDesignsDir(const QString& projectDir);
QString toProjectImportsDir(const QString& projectDir);
QString toProjectAssetsDir(const QString& projectDir);
QString toProjectToolsDir(const QString& projectDir);
QString toProjectMetaDir(const QString& projectDir);
QString toUserProjectsDir(const QString& userDir);
QString toUserMetaDir(const QString& userDir);

QString toControlMainQmlFile(const QString& controlDir);
QString toControlIconFile(const QString& controlDir);
QString toControlMetaFile(const QString& controlDir);
QString toProjectMetaFile(const QString& projectDir);
QString toUserIconFile(const QString& userDir);
QString toUserMetaFile(const QString& userDir);

QString controlId(const QString& controlDir);
QString controlUid(const QString& controlDir);
QString controlToolName(const QString& controlDir);
QString controlToolCategory(const QString& controlDir);
QByteArray controlIcon(const QString& controlDir);

bool projectHdpiScaling(const QString& projectDir);
qint64 projectSize(const QString& projectDir);
QString projectUid(const QString& projectDir);
QString projectName(const QString& projectDir);
QString projectDescription(const QString& projectDir);
QDateTime projectCreationDate(const QString& projectDir);
QDateTime projectModificationDate(const QString& projectDir);
QByteArray projectTheme(const QString& projectDir);

quint32 userPlan(const QString& userDir);
QString userEmail(const QString& userDir);
QString userFirst(const QString& userDir);
QString userLast(const QString& userDir);
QString userCountry(const QString& userDir);
QString userCompany(const QString& userDir);
QString userTitle(const QString& userDir);
QString userPhone(const QString& userDir);
QDateTime userLastOnlineDate(const QString& userDir);
QDateTime userRegistrationDate(const QString& userDir);
QByteArray userPassword(const QString& userDir);
QByteArray userIcon(const QString& userDir);

ControlMetaHash controlMetaHash(const QString& controlDir);
ProjectMetaHash projectMetaHash(const QString& projectDir);
UserMetaHash userMetaHash(const QString& userDir);

QVariant property(const QString& controlDir, ControlProperties property);
QVariant property(const QString& projectDir, ProjectProperties property);
QVariant property(const QString& userDir, UserProperties property);

bool setProperty(const QString& controlDir, ControlProperties property, const QVariant& value);
bool setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value);
bool setProperty(const QString& userDir, UserProperties property, const QVariant& value);

bool initProjectMeta(const QString& projectDir);
bool initUserMeta(const QString& userDir);

bool isForm(const QString& controlDir);
bool isControlValid(const QString& controlDir);
bool isProjectValid(const QString& projectDir);
bool isUserValid(const QString& userDir);

void regenerateUids(const QString& topPath);
QString toDoubleUp(const QString& path);
QStringList formPaths(const QString& projectDir);
QStringList childrenPaths(const QString& controlDir);

} // SaveUtils

#endif // SAVEUTILS_H
