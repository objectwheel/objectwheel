#include <build.h>
#include <projectmanager.h>
#include <filemanager.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#define FILE_NAME "build.json"
#define ICON_NAME "icon.png"
#define KEYSTORE_NAME "keystore.ks"

QString Build::dir()
{
    auto projectDir = ProjectManager::
    projectDirectory(ProjectManager::currentProject());

    if (projectDir.isEmpty())
        return QString();

    return projectDir + separator() + DIR_BUILD;
}

void Build::set(const QString& setting, const QVariant& value)
{
    if (!dir().isEmpty() && mkdir(dir())) {
        auto propertyPath = dir() + separator() + FILE_NAME;
        auto propertyData = rdfile(propertyPath);
        auto obj = QJsonDocument::fromJson(propertyData).object();
        obj[setting] = QJsonValue::fromVariant(value);
        propertyData = QJsonDocument(obj).toJson();
        wrfile(propertyPath, propertyData);
    }
}

void Build::setModule(const QString& module, bool value, bool qt)
{
    if (!dir().isEmpty() && mkdir(dir())) {
        auto propertyPath = dir() + separator() + FILE_NAME;
        auto propertyData = rdfile(propertyPath);
        auto obj = QJsonDocument::fromJson(propertyData).object();
        auto modules = obj[qt ? TAG_QT_MUDULES : TAG_OW_MUDULES].toObject();
        modules[module] = value;
        obj[qt ? TAG_QT_MUDULES : TAG_OW_MUDULES] = modules;
        propertyData = QJsonDocument(obj).toJson();
        wrfile(propertyPath, propertyData);
    }
}

void Build::setIcon(const QString& iconPath)
{
    if (!dir().isEmpty()) {
        auto icnDir = dir() + separator() + ICON_NAME;
        if (rm(icnDir) && !iconPath.isEmpty())
            QFile::copy(iconPath, icnDir);
    }
}

void Build::setKeystore(const QString& keystorePath)
{
    if (!dir().isEmpty()) {
        auto ksDir = dir() + separator() + KEYSTORE_NAME;
        if (rm(ksDir) && !keystorePath.isEmpty())
            QFile::copy(keystorePath, ksDir);
    }
}

void Build::clearPermissions()
{
    if (!dir().isEmpty()) {
        auto propertyPath = dir() + separator() + FILE_NAME;
        auto propertyData = rdfile(propertyPath);
        auto obj = QJsonDocument::fromJson(propertyData).object();
        obj[TAG_PERMISSIONS] = QJsonArray();
        propertyData = QJsonDocument(obj).toJson();
        wrfile(propertyPath, propertyData);
    }
}

void Build::addPermission(const QString& permission)
{
    if (!dir().isEmpty() && mkdir(dir())) {
        auto propertyPath = dir() + separator() + FILE_NAME;
        auto propertyData = rdfile(propertyPath);
        auto obj = QJsonDocument::fromJson(propertyData).object();
        auto permissions = obj[TAG_PERMISSIONS].toArray();
        permissions << permission;
        obj[TAG_PERMISSIONS] = permissions;
        propertyData = QJsonDocument(obj).toJson();
        wrfile(propertyPath, propertyData);
    }
}
