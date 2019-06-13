#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QVariant>

namespace SaveUtils {

enum ControlProperties : quint32 {
    ControlSignature = 0x1000,
    ControlVersion,
    ControlId,
    ControlUid,
    ControlIndex
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

QString controlMainQmlFileName();
QString toControlThisDir(const QString& controlDir);
QString toControlChildrenDir(const QString& controlDir);
QString toProjectDesignsDir(const QString& projectDir);
QString toProjectImportsDir(const QString& projectDir);
QString toProjectAssetsDir(const QString& projectDir);
QString toUserProjectsDir(const QString& userDir);
QString toControlMainQmlFile(const QString& controlDir);
QString toDoubleUp(const QString& path);

bool isForm(const QString& controlDir);
bool isControlValid(const QString& controlDir);
bool isProjectValid(const QString& projectDir);
bool isUserValid(const QString& userDir);

quint32 controlIndex(const QString& controlDir);
QString controlId(const QString& controlDir);
QString controlUid(const QString& controlDir);

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

bool setProperty(const QString& controlDir, ControlProperties property, const QVariant& value);
bool setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value);
bool setProperty(const QString& userDir, UserProperties property, const QVariant& value);

bool initControlMeta(const QString& controlDir);
bool initProjectMeta(const QString& projectDir);
bool initUserMeta(const QString& userDir);
void regenerateUids(const QString& topPath);

QVector<QString> formPaths(const QString& projectDir);
QVector<QString> childrenPaths(const QString& controlDir, bool recursive = true);

} // SaveUtils

#endif // SAVEUTILS_H
