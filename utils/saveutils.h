#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <utils_global.h>
#include <QVariant>

namespace SaveUtils {

enum ControlProperties : quint32 { // ### Application related properties
    ControlSignature = 0x1000,
    ControlVersion,
    ControlId,
    ControlUid,
    ControlIndex
};

enum DesignProperties : quint32 { // ### Designer related properties
    DesignSignature = 0x4000,
    DesignVersion,
    DesignPosition,
    DesignSize
};

enum ProjectProperties : quint32 { // ### Designer related properties
    ProjectSignature = 0x2000,
    ProjectVersion,
    ProjectUid,
    ProjectName,
    ProjectDescription,
    ProjectCreationDate,
    ProjectModificationDate,
    ProjectSize,
    ProjectTheme,      // ### TODO: Marked for removal, qtquickcontrols2.conf takes place
    ProjectHdpiScaling // ### TODO: Marked for removal, move it to qtquickcontrols2.conf, it is a QSettings ini, right? then we should be able to add custom values right?
};

enum UserProperties : quint32 { // ### Designer related properties
    UserSignature = 0x3000,
    UserVersion,
    UserEmail,
    UserPassword,
    UserPlan
};

UTILS_EXPORT QString controlMainQmlFileName();
UTILS_EXPORT QString toControlThisDir(const QString& controlDir);
UTILS_EXPORT QString toControlChildrenDir(const QString& controlDir);
UTILS_EXPORT QString toProjectDesignsDir(const QString& projectDir);
UTILS_EXPORT QString toProjectImportsDir(const QString& projectDir);
UTILS_EXPORT QString toProjectAssetsDir(const QString& projectDir);
UTILS_EXPORT QString toUserProjectsDir(const QString& userDir);
UTILS_EXPORT QString toControlMainQmlFile(const QString& controlDir);
UTILS_EXPORT QString toDoubleUp(const QString& path);

UTILS_EXPORT bool isForm(const QString& controlDir);
UTILS_EXPORT bool isControlValid(const QString& controlDir);
UTILS_EXPORT bool isDesignValid(const QString& controlDir);
UTILS_EXPORT bool isProjectValid(const QString& projectDir);
UTILS_EXPORT bool isUserValid(const QString& userDir);

UTILS_EXPORT quint32 controlIndex(const QString& controlDir);
UTILS_EXPORT QString controlId(const QString& controlDir);
UTILS_EXPORT QString controlUid(const QString& controlDir);
UTILS_EXPORT QPointF designPosition(const QString& controlDir);
UTILS_EXPORT QSizeF designSize(const QString& controlDir);

UTILS_EXPORT bool projectHdpiScaling(const QString& projectDir);
UTILS_EXPORT qint64 projectSize(const QString& projectDir);
UTILS_EXPORT QString projectUid(const QString& projectDir);
UTILS_EXPORT QString projectName(const QString& projectDir);
UTILS_EXPORT QString projectDescription(const QString& projectDir);
UTILS_EXPORT QDateTime projectCreationDate(const QString& projectDir);
UTILS_EXPORT QDateTime projectModificationDate(const QString& projectDir);
UTILS_EXPORT QByteArray projectTheme(const QString& projectDir);

UTILS_EXPORT qint64 userPlan(const QString& userDir);
UTILS_EXPORT QString userEmail(const QString& userDir);
UTILS_EXPORT QByteArray userPassword(const QString& userDir);

UTILS_EXPORT bool setProperty(const QString& controlDir, ControlProperties property, const QVariant& value);
UTILS_EXPORT bool setProperty(const QString& controlDir, DesignProperties property, const QVariant& value);
UTILS_EXPORT bool setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value);
UTILS_EXPORT bool setProperty(const QString& userDir, UserProperties property, const QVariant& value);

UTILS_EXPORT bool initControlMeta(const QString& controlDir);
UTILS_EXPORT bool initDesignMeta(const QString& controlDir);
UTILS_EXPORT bool initProjectMeta(const QString& projectDir);
UTILS_EXPORT bool initUserMeta(const QString& userDir);
UTILS_EXPORT void regenerateUids(const QString& topPath);

UTILS_EXPORT QVector<QString> formPaths(const QString& projectDir);
UTILS_EXPORT QVector<QString> childrenPaths(const QString& controlDir, bool recursive = true);

} // SaveUtils

#endif // SAVEUTILS_H
