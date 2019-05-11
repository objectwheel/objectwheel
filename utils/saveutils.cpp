#include <saveutils.h>
#include <filesystemutils.h>
#include <hashfactory.h>
#include <serializeenum.h>

#include <QDateTime>
#include <QJsonValue>
#include <QSaveFile>
#include <QTimerEvent>

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

QBasicTimer SaveUtils::s_syncTimer;
QMap<QDir, SaveUtils::ControlMap> SaveUtils::s_controlCache;
QMap<QDir, SaveUtils::ProjectMap> SaveUtils::s_projectCache;
QMap<QDir, SaveUtils::UserMap> SaveUtils::s_userCache;

SaveUtils::SaveUtils(QObject* parent) : QObject(parent)
{
    s_syncTimer.start(5000, Qt::VeryCoarseTimer, this);
}

SaveUtils::~SaveUtils()
{
    sync();
}

void SaveUtils::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == s_syncTimer.timerId())
        sync();
    else
        QObject::timerEvent(event);
}

bool SaveUtils::isForm(const QString& controlDir)
{
    QDir dir(controlDir);
    dir.cdUp();
    return QStringLiteral(DIR_DESIGNS) == dir.dirName();
}

bool SaveUtils::isControlValid(const QString& controlDir)
{
    const QString& sign = property(controlDir, ControlSignature).toString();
    return sign == QStringLiteral(SIGN_OWCTRL) && !uid(controlDir).isEmpty();
}

bool SaveUtils::isProjectValid(const QString& projectDir)
{
    const QString& sign = property(projectDir, ProjectSignature).toString();
    return sign == QStringLiteral(SIGN_OWPRJT);
}

bool SaveUtils::isUserValid(const QString& userDir)
{
    const QString& sign = property(userDir, UserSignature).toString();
    return sign == QStringLiteral(SIGN_OWUSER);
}

QString SaveUtils::controlMetaFileName()
{
    static const QString& controlMetaFile = QStringLiteral("control.meta");
    return controlMetaFile;
}

QString SaveUtils::projectMetaFileName()
{
    static const QString& projectMetaFile = QStringLiteral("project.meta");
    return projectMetaFile;
}

QString SaveUtils::userMetaFileName()
{
    static const QString& userMetaFile = QStringLiteral("user.meta");
    return userMetaFile;
}

QString SaveUtils::mainQmlFileName()
{
    static const QString& mainQmlFile = QStringLiteral("main.qml");
    return mainQmlFile;
}

QString SaveUtils::toControlMetaFile(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_THIS) + '/' + controlMetaFileName();
}

QString SaveUtils::toProjectMetaFile(const QString& projectDir)
{
    return projectDir + '/' + projectMetaFileName();
}

QString SaveUtils::toUserMetaFile(const QString& userDir)
{
    return userDir + '/' + userMetaFileName();
}

QString SaveUtils::toMainQmlFile(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_THIS) + '/' + mainQmlFileName();
}

QString SaveUtils::toThisDir(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_THIS);
}

QString SaveUtils::toChildrenDir(const QString& controlDir)
{
    return controlDir + '/' + QStringLiteral(DIR_CHILDREN);
}

QString SaveUtils::toParentDir(const QString& controlDir)
{
    QDir dir(controlDir);
    dir.cdUp();
    dir.cdUp();
    return dir.path();
}

QString SaveUtils::toDesignsDir(const QString& projectDir)
{
    return projectDir + '/' + QStringLiteral(DIR_DESIGNS);
}

QString SaveUtils::toImportsDir(const QString& projectDir)
{
    return projectDir + '/' + QStringLiteral(DIR_IMPORTS);
}

QString SaveUtils::toOwDir(const QString& projectDir)
{
    return toImportsDir(projectDir) + '/' + QStringLiteral(DIR_OW);
}

QString SaveUtils::toGlobalDir(const QString& projectDir)
{
    return toOwDir(projectDir) + '/' + QStringLiteral(DIR_GLOBAL);
}

QString SaveUtils::id(const QString& controlDir)
{
    return property(controlDir, ControlId).toString();
}

QString SaveUtils::uid(const QString& controlDir)
{
    return property(controlDir, ControlUid).toString();
}

QString SaveUtils::name(const QString& controlDir)
{
    return property(controlDir, ControlToolName).toString();
}

QString SaveUtils::category(const QString& controlDir)
{
    return property(controlDir, ControlToolCategory).toString();
}

QByteArray SaveUtils::icon(const QString& controlDir)
{
    return property(controlDir, ControlIcon).toByteArray();
}

bool SaveUtils::projectHdpiScaling(const QString& projectDir)
{
    return property(projectDir, ProjectHdpiScaling).value<bool>();
}

qint64 SaveUtils::projectSize(const QString& projectDir)
{
    return property(projectDir, ProjectSize).value<qint64>();
}

QString SaveUtils::projectUid(const QString& projectDir)
{
    return property(projectDir, ProjectUid).value<QString>();
}

QString SaveUtils::projectName(const QString& projectDir)
{
    return property(projectDir, ProjectName).value<QString>();
}

QString SaveUtils::projectDescription(const QString& projectDir)
{
    return property(projectDir, ProjectDescription).value<QString>();
}

QDateTime SaveUtils::projectCreationDate(const QString& projectDir)
{
    return property(projectDir, ProjectCreationDate).value<QDateTime>();
}

QDateTime SaveUtils::projectModificationDate(const QString& projectDir)
{
    return property(projectDir, ProjectModificationDate).value<QDateTime>();
}

QJsonValue SaveUtils::projectTheme(const QString& projectDir)
{
    return property(projectDir, ProjectTheme).value<QJsonValue>();
}

quint32 SaveUtils::userPlan(const QString& userDir)
{
    return property(userDir, UserPlan).value<quint32>();
}

QString SaveUtils::userEmail(const QString& userDir)
{
    return property(userDir, UserEmail).value<QString>();
}

QString SaveUtils::userFirst(const QString& userDir)
{
    return property(userDir, UserFirst).value<QString>();
}

QString SaveUtils::userLast(const QString& userDir)
{
    return property(userDir, UserLast).value<QString>();
}

QString SaveUtils::userCountry(const QString& userDir)
{
    return property(userDir, UserCountry).value<QString>();
}

QString SaveUtils::userCompany(const QString& userDir)
{
    return property(userDir, UserCompany).value<QString>();
}

QString SaveUtils::userTitle(const QString& userDir)
{
    return property(userDir, UserTitle).value<QString>();
}

QString SaveUtils::userPhone(const QString& userDir)
{
    return property(userDir, UserPhone).value<QString>();
}

QByteArray SaveUtils::userPassword(const QString& userDir)
{
    return property(userDir, UserPassword).value<QByteArray>();
}

QByteArray SaveUtils::userIcon(const QString& userDir)
{
    return property(userDir, UserIcon).value<QByteArray>();
}

QDateTime SaveUtils::userLastOnlineDate(const QString& userDir)
{
    return property(userDir, UserLastOnlineDate).value<QDateTime>();
}

QDateTime SaveUtils::userRegistrationDate(const QString& userDir)
{
    return property(userDir, UserRegistrationDate).value<QDateTime>();
}

SaveUtils::ControlMap SaveUtils::controlMap(const QString& controlDir)
{
    ControlMap map;
    QFile file(toControlMetaFile(controlDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open control meta file");
        return map;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> map;
    file.close();
    return map;
}

SaveUtils::ProjectMap SaveUtils::projectMap(const QString& projectDir)
{
    ProjectMap map;
    QFile file(toProjectMetaFile(projectDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open project meta file");
        return map;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> map;
    file.close();
    return map;
}

SaveUtils::UserMap SaveUtils::userMap(const QString& userDir)
{
    UserMap map;
    QFile file(toUserMetaFile(userDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open user meta file");
        return map;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> map;
    file.close();
    return map;
}

QVariant SaveUtils::property(const QString& controlDir, SaveUtils::ControlProperties property)
{
    return controlMap(controlDir).value(property);
}

QVariant SaveUtils::property(const QString& projectDir, SaveUtils::ProjectProperties property)
{
    return projectMap(projectDir).value(property);
}

QVariant SaveUtils::property(const QString& userDir, SaveUtils::UserProperties property)
{
    return userMap(userDir).value(property);
}

void SaveUtils::setProperty(const QString& controlDir, SaveUtils::ControlProperties property, const QVariant& value)
{
    ControlMap map(controlMap(controlDir));
    map.insert(property, value);
    QSaveFile file(toControlMetaFile(controlDir));
    if (!file.open(QSaveFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open control meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << map;
    if (!file.commit())
        qWarning("SaveUtils: Control meta file save unsuccessful");
}

void SaveUtils::setProperty(const QString& projectDir, SaveUtils::ProjectProperties property, const QVariant& value)
{
    ProjectMap map(projectMap(projectDir));
    map.insert(property, value);
    map.insert(ProjectModificationDate, QDateTime::currentDateTime());
    QSaveFile file(toProjectMetaFile(projectDir));
    if (!file.open(QSaveFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open project meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << map;
    if (!file.commit())
        qWarning("SaveUtils: Project meta file save unsuccessful");
}

void SaveUtils::setProperty(const QString& userDir, SaveUtils::UserProperties property, const QVariant& value)
{
    UserMap map(userMap(userDir));
    map.insert(property, value);
    QSaveFile file(toUserMetaFile(userDir));
    if (!file.open(QSaveFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open user meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << map;
    if (!file.commit())
        qWarning("SaveUtils: User meta file save unsuccessful");
}

void SaveUtils::makeControlMetaFile(const QString& controlDir)
{
    if (!QFileInfo::exists(toControlMetaFile(controlDir))) {
        ControlMap map;
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
        file.close();
    }
}

void SaveUtils::makeProjectMetaFile(const QString& projectDir)
{
    if (!QFileInfo::exists(toProjectMetaFile(projectDir))) {
        ProjectMap map;
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
        file.close();
    }
}

void SaveUtils::makeUserMetaFile(const QString& userDir)
{
    if (!QFileInfo::exists(toUserMetaFile(userDir))) {
        UserMap map;
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
        file.close();
    }
}

void SaveUtils::sync()
{

}

void SaveUtils::regenerateUids(const QString& topPath)
{
    for (const QString& controlFilePath
         : FileSystemUtils::searchFiles(controlMetaFileName(), topPath)) {
        const QString& controlDir = toParentDir(controlFilePath);
        if (!isControlValid(controlDir))
            continue;
        setProperty(controlDir, ControlUid, HashFactory::generate());
    }
}

QStringList SaveUtils::formPaths(const QString& projectDir)
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

QStringList SaveUtils::childrenPaths(const QString& controlDir)
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

