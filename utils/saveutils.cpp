#include <saveutils.h>
#include <filesystemutils.h>
#include <hashfactory.h>
#include <serializeenum.h>

#include <private/qfilesystemengine_p.h>

#include <QJsonValue>
#include <QSaveFile>

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
    return dir.dirName().compare(QLatin1String(DIR_DESIGNS), QFileSystemEngine::isCaseSensitive()
                                 ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0;
}

bool isControlValid(const QString& controlDir)
{
    const QString& sign = property(controlDir, ControlSignature).toString();
    return sign == QLatin1String(SIGN_OWCTRL) && !uid(controlDir).isEmpty();
}

bool isProjectValid(const QString& projectDir)
{
    const QString& sign = property(projectDir, ProjectSignature).toString();
    return sign == QLatin1String(SIGN_OWPRJT);
}

bool isUserValid(const QString& userDir)
{
    const QString& sign = property(userDir, UserSignature).toString();
    return sign == QLatin1String(SIGN_OWUSER);
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

QByteArray userPassword(const QString& userDir)
{
    return property(userDir, UserPassword).value<QByteArray>();
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

SaveUtils::ControlHash controlHash(const QString& controlDir)
{
    ControlHash hash;
    QFile file(toControlMetaFile(controlDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open control meta file");
        return hash;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> hash;
    file.close();
    return hash;
}

SaveUtils::ProjectHash projectHash(const QString& projectDir)
{
    ProjectHash hash;
    QFile file(toProjectMetaFile(projectDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open project meta file");
        return hash;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> hash;
    file.close();
    return hash;
}

SaveUtils::UserHash userHash(const QString& userDir)
{
    UserHash hash;
    QFile file(toUserMetaFile(userDir));
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Cannot open user meta file");
        return hash;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> hash;
    file.close();
    return hash;
}

QVariant property(const QString& controlDir, SaveUtils::ControlProperties property)
{
    return controlHash(controlDir).value(property);
}

QVariant property(const QString& projectDir, SaveUtils::ProjectProperties property)
{
    return projectHash(projectDir).value(property);
}

QVariant property(const QString& userDir, SaveUtils::UserProperties property)
{
    return userHash(userDir).value(property);
}

void setProperty(const QString& controlDir, SaveUtils::ControlProperties property, const QVariant& value)
{
    ControlHash hash(controlHash(controlDir));
    hash.insert(property, value);
    QSaveFile file(toControlMetaFile(controlDir));
    if (!file.open(QSaveFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open control meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << hash;
    if (!file.commit())
        qWarning("SaveUtils: Control meta file save unsuccessful");
}

void setProperty(const QString& projectDir, SaveUtils::ProjectProperties property, const QVariant& value)
{
    ProjectHash hash(projectHash(projectDir));
    hash.insert(property, value);
    hash.insert(ProjectModificationDate, QDateTime::currentDateTime());
    QSaveFile file(toProjectMetaFile(projectDir));
    if (!file.open(QSaveFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open project meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << hash;
    if (!file.commit())
        qWarning("SaveUtils: Project meta file save unsuccessful");
}

void setProperty(const QString& userDir, SaveUtils::UserProperties property, const QVariant& value)
{
    UserHash hash(userHash(userDir));
    hash.insert(property, value);    
    QSaveFile file(toUserMetaFile(userDir));
    if (!file.open(QSaveFile::WriteOnly)) {
        qWarning("SaveUtils: Cannot open user meta file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << hash;
    if (!file.commit())
        qWarning("SaveUtils: User meta file save unsuccessful");
}

void makeControlMetaFile(const QString& controlDir)
{
    if (!QFileInfo::exists(toControlMetaFile(controlDir))) {
        ControlHash hash;
        hash.insert(ControlVersion, qreal(VERSION));
        hash.insert(ControlSignature, QStringLiteral(SIGN_OWCTRL));
        QFile file(toControlMetaFile(controlDir));
        if (!file.open(QFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open control meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << hash;
        file.close();
    }
}

void makeProjectMetaFile(const QString& projectDir)
{
    if (!QFileInfo::exists(toProjectMetaFile(projectDir))) {
        ProjectHash hash;
        hash.insert(ProjectVersion, qreal(VERSION));
        hash.insert(ProjectSignature, QStringLiteral(SIGN_OWPRJT));
        QFile file(toProjectMetaFile(projectDir));
        if (!file.open(QFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open project meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << hash;
        file.close();
    }
}

void makeUserMetaFile(const QString& userDir)
{
    if (!QFileInfo::exists(toUserMetaFile(userDir))) {
        UserHash hash;
        hash.insert(UserVersion, qreal(VERSION));
        hash.insert(UserSignature, QStringLiteral(SIGN_OWUSER));
        QFile file(toUserMetaFile(userDir));
        if (!file.open(QFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open user meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << hash;
        file.close();
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