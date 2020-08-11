/****************************************************************************
**  THE CONTROL MODEL
**
**    users
**        a95db34f1234 // user 1
**        f9713917f242 // user 2
**        52078796f7be // user 3
**            projects
**                906e6f1f676a // project1
**                a0d66c014b35 // project2
**                    build
**                        build.json
**                    meta
**                        project.meta
**                    imports
**                        Assets
**                            qmldir
**                            assets.qml
**                            form1.js
**                            form2.js
**                            form3.js
**                            ... // other resources
**                    designs
**                        fcdb1a5d095d // form1
**                        1ad2135d47fc // form2
**                        4cbcd2ef095d // form3
**                            t
**                                main.qml
**                            m
**                                control.meta
**                            c
**                                63e9fc09515d // child control 1
**                                510c11157413 // child control 2
**                                    t
**                                        main.qml
**                                    m
**                                        control.meta
**                                    c
**                                        2aaa80a57c91 // child control 1
**                                        85024917dacc // child control 2
**                                        ae4ab544d4bc // child control 3
**                                            t
**                                                main.qml
**                                            m
**                                                control.meta
**            meta
**                user.meta
**
****************************************************************************/

#include <saveutils.h>
#include <filesystemutils.h>
#include <hashfactory.h>

#include <QSaveFile>
#include <QDir>
#include <QDateTime>
#include <QDataStream>
#include <QVector>
#include <QPointF>
#include <QSizeF>

namespace SaveUtils {

using ControlMetaHash = QHash<ControlProperties, QVariant>;
using DesignMetaHash = QHash<DesignProperties, QVariant>;
using ProjectMetaHash = QHash<ProjectProperties, QVariant>;
using UserMetaHash = QHash<UserProperties, QVariant>;

namespace Internal {

template <typename MetaHash>
MetaHash readMetaHash(const QString& metaFilePath)
{
    MetaHash hash;
    QFile file(metaFilePath);
    if (!file.open(QFile::ReadOnly)) {
#if defined(VERBOSE_READ)
        qWarning("SaveUtils: Failed to read meta file, %s", metaFilePath.toUtf8().constData());
#endif
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

double version()
{
    return 3.0;
}

QString controlSignature()
{
    return QStringLiteral("b3djdHJs");
}

QString designSignature()
{
    return QStringLiteral("b3dkc2du");
}

QString projectSignature()
{
    return QStringLiteral("b3dwcmp0");
}

QString userSignature()
{
    return QStringLiteral("b3d1c2Vy");
}

QString controlMetaFileName()
{
    return QStringLiteral("control.meta");
}

QString designMetaFileName()
{
    return QStringLiteral("design.meta");
}

QString projectMetaFileName()
{
    return QStringLiteral("project.meta");
}

QString userMetaFileName()
{
    return QStringLiteral("user.meta");
}

QString toControlMetaDir(const QString& controlDir)
{
    return controlDir + QStringLiteral("/m");
}

QString toProjectMetaDir(const QString& projectDir)
{
    return projectDir + QStringLiteral("/meta");
}

QString toUserMetaDir(const QString& userDir)
{
    return userDir + QStringLiteral("/meta");
}

QString toControlMetaFile(const QString& controlDir)
{
    return toControlMetaDir(controlDir) + '/' + controlMetaFileName();
}

QString toDesignMetaFile(const QString& controlDir)
{
    return toControlMetaDir(controlDir) + '/' + designMetaFileName();
}

QString toProjectMetaFile(const QString& projectDir)
{
    return toProjectMetaDir(projectDir) + '/' + projectMetaFileName();
}

QString toUserMetaFile(const QString& userDir)
{
    return toUserMetaDir(userDir) + '/' + userMetaFileName();
}

ControlMetaHash controlMetaHash(const QString& controlDir)
{
    return readMetaHash<ControlMetaHash>(toControlMetaFile(controlDir));
}

DesignMetaHash designMetaHash(const QString& controlDir)
{
    return readMetaHash<DesignMetaHash>(toDesignMetaFile(controlDir));
}

ProjectMetaHash projectMetaHash(const QString& projectDir)
{
    return readMetaHash<ProjectMetaHash>(toProjectMetaFile(projectDir));
}

UserMetaHash userMetaHash(const QString& userDir)
{
    return readMetaHash<UserMetaHash>(toUserMetaFile(userDir));
}

QVariant property(const QString& controlDir, ControlProperties property)
{
    return controlMetaHash(controlDir).value(property);
}

QVariant property(const QString& controlDir, DesignProperties property)
{
    return designMetaHash(controlDir).value(property);
}

QVariant property(const QString& projectDir, ProjectProperties property)
{
    return projectMetaHash(projectDir).value(property);
}

QVariant property(const QString& userDir, UserProperties property)
{
    return userMetaHash(userDir).value(property);
}

} // Internal

QString controlMainQmlFileName()
{
    return QStringLiteral("main.qml");
}

QString toControlThisDir(const QString& controlDir)
{
    return controlDir + QStringLiteral("/t");
}

QString toControlChildrenDir(const QString& controlDir)
{
    return controlDir + QStringLiteral("/c");
}

QString toProjectDesignsDir(const QString& projectDir)
{
    return projectDir + QStringLiteral("/designs");
}

QString toProjectImportsDir(const QString& projectDir)
{
    return projectDir + QStringLiteral("/imports");
}

QString toProjectAssetsDir(const QString& projectDir)
{
    return toProjectImportsDir(projectDir) + QStringLiteral("/Assets");
}

QString toUserProjectsDir(const QString& userDir)
{
    return userDir + QStringLiteral("/projects");
}

QString toControlMainQmlFile(const QString& controlDir)
{
    return toControlThisDir(controlDir) + '/' + controlMainQmlFileName();
}

QString toDoubleUp(const QString& path)
{
    Q_ASSERT(QFileInfo::exists(path));
    QDir dir(QFileInfo(path).dir());
    dir.cdUp();
    return dir.path();
}

bool isForm(const QString& controlDir)
{
    QDir dir(controlDir);
    dir.cdUp();
    return dir.dirName() == QLatin1String("designs");
}

bool isControlValid(const QString& controlDir)
{
    return Internal::property(controlDir, ControlSignature).toString() == Internal::controlSignature()
            && !controlUid(controlDir).isEmpty();
}

bool isDesignValid(const QString& controlDir)
{
    return Internal::property(controlDir, DesignSignature).toString() == Internal::designSignature();
}

bool isProjectValid(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectSignature).toString() == Internal::projectSignature()
            && !projectUid(projectDir).isEmpty();
}

bool isUserValid(const QString& userDir)
{
    return Internal::property(userDir, UserSignature).toString() == Internal::userSignature();
}

quint32 controlIndex(const QString& controlDir)
{
    return Internal::property(controlDir, ControlIndex).value<quint32>();
}

QString controlId(const QString& controlDir)
{
    return Internal::property(controlDir, ControlId).value<QString>();
}

QString controlUid(const QString& controlDir)
{
    return Internal::property(controlDir, ControlUid).value<QString>();
}

QPointF designPosition(const QString& controlDir)
{
    return Internal::property(controlDir, DesignPosition).value<QPointF>();
}

QSizeF designSize(const QString& controlDir)
{
    return Internal::property(controlDir, DesignSize).value<QSizeF>();
}

bool projectHdpiScaling(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectHdpiScaling).value<bool>();
}

qint64 projectSize(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectSize).value<qint64>();
}

QString projectUid(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectUid).value<QString>();
}

QString projectName(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectName).value<QString>();
}

QString projectDescription(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectDescription).value<QString>();
}

QDateTime projectCreationDate(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectCreationDate).value<QDateTime>();
}

QDateTime projectModificationDate(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectModificationDate).value<QDateTime>();
}

QByteArray projectTheme(const QString& projectDir)
{
    return Internal::property(projectDir, ProjectTheme).value<QByteArray>();
}

qint64 userPlan(const QString& userDir)
{
    return Internal::property(userDir, UserPlan).value<qint64>();
}

QString userEmail(const QString& userDir)
{
    return Internal::property(userDir, UserEmail).value<QString>();
}

QByteArray userPassword(const QString& userDir)
{
    return Internal::property(userDir, UserPassword).value<QByteArray>();
}

bool setProperty(const QString& controlDir, ControlProperties property, const QVariant& value)
{
    ControlMetaHash hash(Internal::controlMetaHash(controlDir));
    hash.insert(property, value);
    return Internal::saveMetaHash(hash, Internal::toControlMetaFile(controlDir));
}

bool setProperty(const QString& controlDir, DesignProperties property, const QVariant& value)
{
    DesignMetaHash hash(Internal::designMetaHash(controlDir));
    hash.insert(property, value);
    return Internal::saveMetaHash(hash, Internal::toDesignMetaFile(controlDir));
}

bool setProperty(const QString& projectDir, ProjectProperties property, const QVariant& value)
{
    ProjectMetaHash hash(Internal::projectMetaHash(projectDir));
    hash.insert(property, value);
    hash.insert(ProjectModificationDate, QDateTime::currentDateTime());
    return Internal::saveMetaHash(hash, Internal::toProjectMetaFile(projectDir));
}

bool setProperty(const QString& userDir, UserProperties property, const QVariant& value)
{
    UserMetaHash hash(Internal::userMetaHash(userDir));
    hash.insert(property, value);
    return Internal::saveMetaHash(hash, Internal::toUserMetaFile(userDir));
}

bool initControlMeta(const QString& controlDir)
{
    if (QFileInfo::exists(Internal::toControlMetaFile(controlDir)))
        return true;

    if (!QDir(Internal::toControlMetaDir(controlDir)).mkpath(QStringLiteral(".")))
        return false;

    ControlMetaHash hash;
    hash.insert(ControlSignature, Internal::controlSignature());
    hash.insert(ControlVersion, Internal::version());
    hash.insert(ControlUid, HashFactory::generate());

    return Internal::saveMetaHash(hash, Internal::toControlMetaFile(controlDir));
}

bool initDesignMeta(const QString& controlDir)
{
    if (QFileInfo::exists(Internal::toDesignMetaFile(controlDir)))
        return true;

    if (!QDir(Internal::toControlMetaDir(controlDir)).mkpath(QStringLiteral(".")))
        return false;

    DesignMetaHash hash;
    hash.insert(DesignSignature, Internal::designSignature());
    hash.insert(DesignVersion, Internal::version());

    return Internal::saveMetaHash(hash, Internal::toDesignMetaFile(controlDir));
}

bool initProjectMeta(const QString& projectDir)
{
    if (QFileInfo::exists(Internal::toProjectMetaFile(projectDir)))
        return true;

    if (!QDir(Internal::toProjectMetaDir(projectDir)).mkpath(QStringLiteral(".")))
        return false;

    ProjectMetaHash hash;
    hash.insert(ProjectSignature, Internal::projectSignature());
    hash.insert(ProjectVersion, Internal::version());
    hash.insert(ProjectUid, HashFactory::generate());
    hash.insert(ProjectHdpiScaling, true);

    return Internal::saveMetaHash(hash, Internal::toProjectMetaFile(projectDir));
}

bool initUserMeta(const QString& userDir)
{
    if (QFileInfo::exists(Internal::toUserMetaFile(userDir)))
        return true;

    if (!QDir(Internal::toUserMetaDir(userDir)).mkpath(QStringLiteral(".")))
        return false;

    UserMetaHash hash;
    hash.insert(UserSignature, Internal::userSignature());
    hash.insert(UserVersion, Internal::version());
    return Internal::saveMetaHash(hash, Internal::toUserMetaFile(userDir));
}

void regenerateUids(const QString& topPath)
{
    foreach (const QString& controlFilePath,
             FileSystemUtils::searchFiles(Internal::controlMetaFileName(), topPath)) {
        const QString& controlDir = toDoubleUp(controlFilePath);
        if (!isControlValid(controlDir)) {
            Q_ASSERT(Internal::property(controlDir, ControlSignature).toString() != Internal::controlSignature());
            continue;
        }
        setProperty(controlDir, ControlUid, HashFactory::generate());
    }
}

QVector<QString> formPaths(const QString& projectDir)
{
    if (!isProjectValid(projectDir)) {
        qWarning("SaveUtils: Invalid project dir has given");
        return {};
    }

    QVector<QString> paths;
    const QString& designsDir = toProjectDesignsDir(projectDir);
    foreach (const QString& formDirName, QDir(designsDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString& formDir = designsDir + '/' + formDirName;
        if (isControlValid(formDir))
            paths.append(formDir);
    }

    std::sort(paths.begin(), paths.end(), [] (const QString& left, const QString& right) {
        return controlIndex(left) < controlIndex(right);
    });

    return paths;
}

QVector<QString> childrenPaths(const QString& controlDir, bool recursive)
{
    if (!isControlValid(controlDir)) {
        qWarning("SaveUtils: Invalid control dir has given");
        return {};
    }

    QVector<QString> paths;
    const QString& childrenDir = toControlChildrenDir(controlDir);
    foreach (const QString& childDirName, QDir(childrenDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString& childControlDir = childrenDir + '/' + childDirName;
        if (isControlValid(childControlDir))
            paths.append(childControlDir);
    }

    std::sort(paths.begin(), paths.end(), [] (const QString& left, const QString& right) {
        return controlIndex(left) < controlIndex(right);
    });

    if (recursive) {
        const int SIBLINGS_COUNT = paths.size();
        for (int i = 0; i < SIBLINGS_COUNT; ++i)
            paths.append(childrenPaths(paths.at(i), true));
    }

    return paths;
}

} // SaveUtils
