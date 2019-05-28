#include <saveutils.h>
#include <filesystemutils.h>
#include <hashfactory.h>
#include <serializeenum.h>

#include <QSaveFile>
#include <QDir>
#include <QDateTime>
#include <QDataStream>

namespace SaveUtils {

namespace Internal {

constexpr double version()
{
    return 3.0;
}

QString controlSignature()
{
    return QStringLiteral("b3djdHJs");
}

QString projectSignature()
{
    return QStringLiteral("b3dwcmp0");
}

QString userSignature()
{
    return QStringLiteral("b3d1c2Vy");
}

QByteArray readFile(const QString& filePath)
{
    QByteArray data;
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Failed to read file, %s", filePath.toUtf8().constData());
        return data;
    }
    data = file.readAll();
    file.close();
    return data;
}

template <typename MetaHash>
MetaHash readMetaHash(const QString& metaFilePath)
{
    MetaHash hash;
    QFile file(metaFilePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveUtils: Failed to read meta file, %s", metaFilePath.toUtf8().constData());
        return hash;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_12);
    in >> hash;
    file.close();
    return hash;
}

template <typename MetaHash>
bool saveMetaHash(const MetaHash& hash, const QString& metaFilePath)
{
    QSaveFile file(metaFilePath);
    if (!file.open(QSaveFile::WriteOnly)) {
        qWarning("SaveUtils: Failed to open meta file, %s", metaFilePath.toUtf8().constData());
        return false;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);
    out << hash;
    if (!file.commit()) {
        qWarning("SaveUtils: Failed to save meta file, %s", metaFilePath.toUtf8().constData());
        return false;
    }
    return true;
}

} // Internal

QString controlMainQmlFileName()
{
    return QStringLiteral("main.qml");
}

QString controlIconFileName()
{
    return QStringLiteral("icon");
}

QString controlMetaFileName()
{
    return QStringLiteral("control.meta");
}

QString projectMetaFileName()
{
    return QStringLiteral("project.meta");
}

QString userIconFileName()
{
    return QStringLiteral("icon");
}

QString userMetaFileName()
{
    return QStringLiteral("user.meta");
}

QString toControlThisDir(const QString& controlDir)
{
    return controlDir + QStringLiteral("/t");
}

QString toControlChildrenDir(const QString& controlDir)
{
    return controlDir + QStringLiteral("/c");
}

QString toControlMetaDir(const QString& controlDir)
{
    return controlDir + QStringLiteral("/m");
}

QString toProjectDesignsDir(const QString& projectDir)
{
    return projectDir + QStringLiteral("/designs");
}

QString toProjectAssetsDir(const QString& projectDir)
{
    return projectDir + QStringLiteral("/assets");
}

QString toProjectMetaDir(const QString& projectDir)
{
    return projectDir + QStringLiteral("/meta");
}

QString toUserProjectsDir(const QString& userDir)
{
    return userDir + QStringLiteral("/projects");
}

QString toUserMetaDir(const QString& userDir)
{
    return userDir + QStringLiteral("/meta");
}

QString toControlMainQmlFile(const QString& controlDir)
{
    return toControlThisDir(controlDir) + '/' + controlMainQmlFileName();
}

QString toControlIconFile(const QString& controlDir)
{
    return toControlMetaDir(controlDir) + '/' + controlIconFileName();
}

QString toControlMetaFile(const QString& controlDir)
{
    return toControlMetaDir(controlDir) + '/' + controlMetaFileName();
}

QString toProjectMetaFile(const QString& projectDir)
{
    return toProjectMetaDir(projectDir) + '/' + projectMetaFileName();
}

QString toUserIconFile(const QString& userDir)
{
    return toUserMetaDir(userDir) + '/' + userIconFileName();
}

QString toUserMetaFile(const QString& userDir)
{
    return toUserMetaDir(userDir) + '/' + userMetaFileName();
}

QString controlId(const QString& controlDir)
{
    return property(controlDir, ControlId).toString();
}

QString controlUid(const QString& controlDir)
{
    return property(controlDir, ControlUid).toString();
}

QString controlToolName(const QString& controlDir)
{
    return property(controlDir, ControlToolName).toString();
}

QString controlToolCategory(const QString& controlDir)
{
    return property(controlDir, ControlToolCategory).toString();
}

QByteArray controlIcon(const QString& controlDir)
{
    return Internal::readFile(toControlIconFile(controlDir));
}

bool projectHdpiScaling(const QString& projectDir)
{
    return property(projectDir, ProjectHdpiScaling).value<bool>();
}

qint64 projectSize(const QString& projectDir)
{
    return property(projectDir, ProjectSize).value<qint64>();
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

QByteArray projectTheme(const QString& projectDir)
{
    return property(projectDir, ProjectTheme).value<QByteArray>();
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

QDateTime userLastOnlineDate(const QString& userDir)
{
    return property(userDir, UserLastOnlineDate).value<QDateTime>();
}

QDateTime userRegistrationDate(const QString& userDir)
{
    return property(userDir, UserRegistrationDate).value<QDateTime>();
}

QByteArray userPassword(const QString& userDir)
{
    return property(userDir, UserPassword).value<QByteArray>();
}

QByteArray userIcon(const QString& userDir)
{
    return Internal::readFile(toUserIconFile(userDir));
}

ControlMetaHash controlMetaHash(const QString& controlDir)
{
    return Internal::readMetaHash<ControlMetaHash>(toControlMetaFile(controlDir));
}

ProjectMetaHash projectMetaHash(const QString& projectDir)
{
    return Internal::readMetaHash<ProjectMetaHash>(toProjectMetaFile(projectDir));
}

UserMetaHash userMetaHash(const QString& userDir)
{
    return Internal::readMetaHash<UserMetaHash>(toUserMetaFile(userDir));
}

QVariant property(const QString& controlDir, ControlProperties property)
{
    return controlMetaHash(controlDir).value(property);
}

QVariant property(const QString& projectDir, ProjectProperties property)
{
    return projectMetaHash(projectDir).value(property);
}

QVariant property(const QString& userDir, UserProperties property)
{
    return userMetaHash(userDir).value(property);
}

bool setProperty(const QString& controlDir, ControlProperties property, const QVariant& value)
{
    ControlMetaHash hash(controlMetaHash(controlDir));
    hash.insert(property, value);
    return Internal::saveMetaHash(hash, toControlMetaFile(controlDir));
}

bool setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value)
{
    ProjectMetaHash hash(projectMetaHash(projectDir));
    hash.insert(property, value);
    hash.insert(ProjectModificationDate, QDateTime::currentDateTime());
    return Internal::saveMetaHash(hash, toProjectMetaFile(projectDir));
}

bool setProperty(const QString& userDir, UserProperties property, const QVariant& value)
{
    UserMetaHash hash(userMetaHash(userDir));
    hash.insert(property, value);
    return Internal::saveMetaHash(hash, toUserMetaFile(userDir));
}

bool makeProjectMeta(const QString& projectDir)
{
    if (QFileInfo::exists(toProjectMetaFile(projectDir)))
        return true;

    if (!QDir(toProjectMetaDir(projectDir)).mkpath(QStringLiteral(".")))
        return false;

    ProjectMetaHash hash;
    hash.insert(ProjectSignature, Internal::projectSignature());
    hash.insert(ProjectVersion, Internal::version());
    return Internal::saveMetaHash(hash, toProjectMetaFile(projectDir));
}

bool makeUserMeta(const QString& userDir)
{
    if (QFileInfo::exists(toUserMetaFile(userDir)))
        return true;

    if (!QDir(toUserMetaDir(userDir)).mkpath(QStringLiteral(".")))
        return false;

    UserMetaHash hash;
    hash.insert(UserSignature, Internal::userSignature());
    hash.insert(UserVersion, Internal::version());
    return Internal::saveMetaHash(hash, toUserMetaFile(userDir));
}

bool isForm(const QString& controlDir)
{
    QDir dir(controlDir);
    dir.cdUp();
    return dir.dirName() == QLatin1String("designs");
}

bool isControlValid(const QString& controlDir)
{
    return property(controlDir, ControlSignature).toString() == Internal::controlSignature()
            && !controlUid(controlDir).isEmpty();
}

bool isProjectValid(const QString& projectDir)
{
    return property(projectDir, ProjectSignature).toString() == Internal::projectSignature();
}

bool isUserValid(const QString& userDir)
{
    return property(userDir, UserSignature).toString() == Internal::userSignature();
}

void regenerateUids(const QString& topPath)
{
    for (const QString& controlFilePath
         : FileSystemUtils::searchFiles(controlMetaFileName(), topPath)) {
        const QString& controlDir = toDoubleUp(controlFilePath);
        if (!isControlValid(controlDir)) {
            Q_ASSERT(property(controlDir, ControlSignature).toString() != Internal::controlSignature());
            continue;
        }
        setProperty(controlDir, ControlUid, HashFactory::generate());
    }
}

QString toDoubleUp(const QString& path)
{
    Q_ASSERT(QFileInfo::exists(path));
    QDir dir(QFileInfo(path).dir());
    dir.cdUp();
    return dir.path();
}

QStringList formPaths(const QString& projectDir)
{
    QStringList paths;
    const QString& designsDir = toProjectDesignsDir(projectDir);
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
    const QString& childrenDir = toControlChildrenDir(controlDir);
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