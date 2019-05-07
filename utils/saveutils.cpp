#include <saveutils.h>
#include <filesystemutils.h>
#include <hashfactory.h>
#include <serializeenum.h>

#include <QDateTime>
#include <QDir>
#include <QJsonValue>

#define VERSION      2.9
#define SIGN_OWCTRL  "b3djdHJs"
#define SIGN_OWPRJT  "b3dwcmp0"
#define SIGN_OWUSER  "b3d1c2Vy"
#define DIR_THIS     "t"
#define DIR_CHILDREN "c"
#define DIR_DESIGNS  "designs"
#define DIR_IMPORTS  "imports"
#define DIR_OW       "Objectwheel"
#define DIR_GLOBAL   "GlobalResources"

namespace SaveUtils {

bool isForm(const QString& controlDir)
{
    QDir dir(controlDir);
    dir.cdUp();
    return QStringLiteral(DIR_DESIGNS) == dir.dirName();
}

bool isControlValid(const QString& controlDir)
{
    const QString& sign = property(controlDir, ControlSignature).toString();
    return sign == QStringLiteral(SIGN_OWCTRL) && !uid(controlDir).isEmpty();
}

bool isProjectValid(const QString& projectDir)
{
    const QString& sign = property(projectDir, ProjectSignature).toString();
    return sign == QStringLiteral(SIGN_OWPRJT);
}

bool isUserValid(const QString& userDir)
{
    const QString& sign = property(userDir, UserSignature).toString();
    return sign == QStringLiteral(SIGN_OWUSER);
}

QString controlMetaFileName()
{
    static const QString& controlMetaFile = QStringLiteral("control.meta");
    return controlMetaFile;
}

QString projectMetaFileName()
{
    static const QString& projectMetaFile = QStringLiteral("project.meta");
    return projectMetaFile;
}

QString userMetaFileName()
{
    static const QString& userMetaFile = QStringLiteral("user.meta");
    return userMetaFile;
}

QString mainQmlFileName()
{
    static const QString& mainQmlFile = QStringLiteral("main.qml");
    return mainQmlFile;
}

QString toControlMetaFile(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_THIS) + '/' + controlMetaFileName();
}

QString toProjectMetaFile(const QString& projectDir)
{
    return projectDir + '/' + projectMetaFileName();
}

QString toUserMetaFile(const QString& userDir)
{
    return userDir + '/' + userMetaFileName();
}

QString toMainQmlFile(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_THIS) + '/' + mainQmlFileName();
}

QString toThisDir(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_THIS);
}

QString toChildrenDir(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_CHILDREN);
}

QString toParentDir(const QString& controlDir)
{
    QDir dir(controlDir);
    dir.cdUp();
    dir.cdUp();
    return dir.path();
}

QString toDesignsDir(const QString& projectDir)
{
    return projectDir + '/' + QStringLiteral(DIR_DESIGNS);
}

QString toImportsDir(const QString& projectDir)
{
    return projectDir + '/' + QStringLiteral(DIR_IMPORTS);
}

QString toOwDir(const QString& projectDir)
{
    return toImportsDir(projectDir) + '/' + QStringLiteral(DIR_OW);
}

QString toGlobalDir(const QString& projectDir)
{
    return toOwDir(projectDir) + '/' + QStringLiteral(DIR_GLOBAL);
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

bool projectHdpiScaling(const QString& projectDir)
{
    return property(projectDir, ProjectHdpiScaling).value<bool>();
}

qint64 projectSize(const QString& projectDir)
{
    return property(projectDir, ProjectSize).value<qint64>();
}

QString projectUid(const QString& projectDir)
{
    return property(projectDir, ProjectUid).value<QString>();
}

QString projectName(const QString& projectDir)
{
    return property(projectDir, ProjectName).value<QString>();
}

QString projectDescription(const QString& projectDir)
{
    return property(projectDir, ProjectDescription).value<QString>();
}

QDateTime projectCreationDate(const QString& projectDir)
{
    return property(projectDir, ProjectCreationDate).value<QDateTime>();
}

QDateTime projectModificationDate(const QString& projectDir)
{
    return property(projectDir, ProjectModificationDate).value<QDateTime>();
}

QJsonValue projectTheme(const QString& projectDir)
{
    return property(projectDir, ProjectTheme).value<QJsonValue>();
}

quint32 userPlan(const QString& userDir)
{
    return property(userDir, UserPlan).value<quint32>();
}

QString userEmail(const QString& userDir)
{
    return property(userDir, UserEmail).value<QString>();
}

QString userFirst(const QString& userDir)
{
    return property(userDir, UserFirst).value<QString>();
}

QString userLast(const QString& userDir)
{
    return property(userDir, UserLast).value<QString>();
}

QString userCountry(const QString& userDir)
{
    return property(userDir, UserCountry).value<QString>();
}

QString userCompany(const QString& userDir)
{
    return property(userDir, UserCompany).value<QString>();
}

QString userTitle(const QString& userDir)
{
    return property(userDir, UserTitle).value<QString>();
}

QString userPhone(const QString& userDir)
{
    return property(userDir, UserPhone).value<QString>();
}

QByteArray userHash(const QString& userDir)
{
    return property(userDir, UserHash).value<QByteArray>();
}

QByteArray userIcon(const QString& userDir)
{
    return property(userDir, UserIcon).value<QByteArray>();
}

QDateTime userLastOnlineDate(const QString& userDir)
{
    return property(userDir, UserLastOnlineDate).value<QDateTime>();
}

QDateTime userRegistrationDate(const QString& userDir)
{
    return property(userDir, UserRegistrationDate).value<QDateTime>();
}

QMap<ControlProperties, QVariant> controlMap(const QString& controlDir)
{
    QMap<ControlProperties, QVariant> map;
    QFile file(toControlMetaFile(controlDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open control meta file");
        return map;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> map;
    return map;
}

QMap<ProjectProperties, QVariant> projectMap(const QString& projectDir)
{
    QMap<ProjectProperties, QVariant> map;
    QFile file(toProjectMetaFile(projectDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open project meta file");
        return map;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> map;
    return map;
}

QMap<UserProperties, QVariant> userMap(const QString& userDir)
{
    QMap<UserProperties, QVariant> map;
    QFile file(toUserMetaFile(userDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open user meta file");
        return map;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
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

QVariant property(const QString& userDir, UserProperties property)
{
    return userMap(userDir).value(property);
}

void setProperty(const QString& controlDir, ControlProperties property, const QVariant& value)
{
    QMap<ControlProperties, QVariant> map(controlMap(controlDir));
    map.insert(property, value);
    QFile file(toControlMetaFile(controlDir));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open control meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << map;
}

void setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value)
{
    QMap<ProjectProperties, QVariant> map(projectMap(projectDir));
    map.insert(property, value);
    map.insert(ProjectModificationDate, QDateTime::currentDateTime());
    QFile file(toProjectMetaFile(projectDir));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open project meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << map;
}

void setProperty(const QString& userDir, UserProperties property, const QVariant& value)
{
    QMap<UserProperties, QVariant> map(userMap(userDir));
    map.insert(property, value);
    QFile file(toUserMetaFile(userDir));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open user meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << map;
}

void makeControlMetaFile(const QString& controlDir)
{
    if (!QFileInfo::exists(toControlMetaFile(controlDir))) {
        QMap<ControlProperties, QVariant> map;
        map.insert(ControlVersion, qreal(VERSION));
        map.insert(ControlSignature, QStringLiteral(SIGN_OWCTRL));
        QFile file(toControlMetaFile(controlDir));
        if (!file.open(QFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open control meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << map;
    }
}

void makeProjectMetaFile(const QString& projectDir)
{
    if (!QFileInfo::exists(toProjectMetaFile(projectDir))) {
        QMap<ProjectProperties, QVariant> map;
        map.insert(ProjectVersion, qreal(VERSION));
        map.insert(ProjectSignature, QStringLiteral(SIGN_OWPRJT));
        QFile file(toProjectMetaFile(projectDir));
        if (!file.open(QFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open project meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << map;
    }
}

void makeUserMetaFile(const QString& userDir)
{
    if (!QFileInfo::exists(toUserMetaFile(userDir))) {
        QMap<UserProperties, QVariant> map;
        map.insert(UserVersion, qreal(VERSION));
        map.insert(UserSignature, QStringLiteral(SIGN_OWUSER));
        QFile file(toUserMetaFile(userDir));
        if (!file.open(QFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open user meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << map;
    }
}

void regenerateUids(const QString& topPath)
{
    for (const QString& controlFilePath
         : FileSystemUtils::searchFiles(controlMetaFileName(), topPath)) {
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
    for (const QString& formDirName
         : QDir(designsDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& formDir = designsDir + '/' + formDirName;
        if (isControlValid(formDir))
            paths.append(formDir);
    }
    return paths;
}

QStringList childrenPaths(const QString& controlDir)
{
    QStringList paths;
    const QString& childrenDir = toChildrenDir(controlDir);
    for (const QString& childDirName
         : QDir(childrenDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString& childControlDir = childrenDir + '/' + childDirName;
        if (isControlValid(childControlDir)) {
            paths.append(childControlDir);
            paths.append(childrenPaths(childControlDir));
        }
    }
    return paths;
}

} // SaveUtils
