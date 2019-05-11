#include <saveutils.h>
#include <filesystemutils.h>
#include <hashfactory.h>
#include <serializeenum.h>

#include <private/qfilesystemengine_p.h>

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
SaveUtils::ControlCache SaveUtils::s_controlCache;
SaveUtils::ProjectCache SaveUtils::s_projectCache;
SaveUtils::UserCache SaveUtils::s_userCache;

SaveUtils::SaveUtils(QObject* parent) : QObject(parent)
{
    s_syncTimer.start(5000, Qt::VeryCoarseTimer, this);
}
#include <QDebug>
SaveUtils::~SaveUtils()
{
    qDebug() << "asdasdasdasd";

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

SaveUtils::ControlHash& SaveUtils::controlHash(const QString& controlDir)
{
    const QDir dir(controlDir);
    if (s_controlCache.contains(dir)) {
        return s_controlCache[dir];
    } else {
        QFile file(toControlMetaFile(controlDir));
        if (!file.open(QFile::ReadOnly)) {
            qWarning("SaveUtils: Cannot open control meta file");
            return s_controlCache[dir];
        }
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_12);
        in >> s_controlCache[dir];
        file.close();
        return s_controlCache[dir];
    }
}

SaveUtils::ProjectHash& SaveUtils::projectHash(const QString& projectDir)
{
    const QDir dir(projectDir);
    if (s_projectCache.contains(dir)) {
        return s_projectCache[dir];
    } else {
        QFile file(toProjectMetaFile(projectDir));
        if (!file.open(QFile::ReadOnly)) {
            qWarning("SaveUtils: Cannot open project meta file");
            return s_projectCache[dir];
        }
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_12);
        in >> s_projectCache[dir];
        file.close();
        return s_projectCache[dir];
    }
}

SaveUtils::UserHash& SaveUtils::userHash(const QString& userDir)
{
    const QDir dir(userDir);
    if (s_userCache.contains(dir)) {
        return s_userCache[dir];
    } else {
        QFile file(toUserMetaFile(userDir));
        if (!file.open(QFile::ReadOnly)) {
            qWarning("SaveUtils: Cannot open user meta file");
            return s_userCache[dir];
        }
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_12);
        in >> s_userCache[dir];
        file.close();
        return s_userCache[dir];
    }
}

QVariant SaveUtils::property(const QString& controlDir, SaveUtils::ControlProperties property)
{
    return controlHash(controlDir).value(property);
}

QVariant SaveUtils::property(const QString& projectDir, SaveUtils::ProjectProperties property)
{
    return projectHash(projectDir).value(property);
}

QVariant SaveUtils::property(const QString& userDir, SaveUtils::UserProperties property)
{
    return userHash(userDir).value(property);
}

void SaveUtils::setProperty(const QString& controlDir, SaveUtils::ControlProperties property, const QVariant& value)
{
    ControlHash& hash(controlHash(controlDir));
    hash.insert(property, value);
}

void SaveUtils::setProperty(const QString& projectDir, SaveUtils::ProjectProperties property, const QVariant& value)
{
    ProjectHash& hash(projectHash(projectDir));
    hash.insert(property, value);
    hash.insert(ProjectModificationDate, QDateTime::currentDateTime());
}

void SaveUtils::setProperty(const QString& userDir, SaveUtils::UserProperties property, const QVariant& value)
{
    UserHash& hash(userHash(userDir));
    hash.insert(property, value);
}

void SaveUtils::makeControlMetaFile(const QString& controlDir)
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

void SaveUtils::makeProjectMetaFile(const QString& projectDir)
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

void SaveUtils::makeUserMetaFile(const QString& userDir)
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

void SaveUtils::sync()
{
    for (const QDir& userDir : s_userCache.keys()) {
        QSaveFile file(toUserMetaFile(userDir.path()));
        if (!file.open(QSaveFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open user meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << s_userCache.value(userDir);
        if (!file.commit())
            qWarning("SaveUtils: User meta file save unsuccessful");
    }

    for (const QDir& projectDir : s_projectCache.keys()) {
        QSaveFile file(toProjectMetaFile(projectDir.path()));
        if (!file.open(QSaveFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open project meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << s_projectCache.value(projectDir);
        if (!file.commit())
            qWarning("SaveUtils: Project meta file save unsuccessful");
    }

    for (const QDir& controlDir : s_controlCache.keys()) {
        QSaveFile file(toControlMetaFile(controlDir.path()));
        if (!file.open(QSaveFile::WriteOnly)) {
            qWarning("SaveUtils: Cannot open control meta file");
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_12);
        out << s_controlCache.value(controlDir);
        if (!file.commit())
            qWarning("SaveUtils: Control meta file save unsuccessful");
    }
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

uint qHash(const QDir& key, uint seed)
{
    // Filters, sort order, nameFilters are ignored
    if (QFileSystemEngine::isCaseSensitive()) {
        if (key.exists())
            return qHash(key.canonicalPath(), seed);
        else
            return qHash(key.absolutePath(), seed);
    } else {
        if (key.exists())
            return qHash(key.canonicalPath().toLower(), seed);
        else
            return qHash(key.absolutePath().toLower(), seed);
    }
}
