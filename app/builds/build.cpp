#include <build.h>
#include <projectmanager.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QImage>

#define FILE_NAME "build.json"
#define KEYSTORE_NAME "keystore.ks"

QString Build::dir()
{
// FIXME   auto projectDir = ProjectManager::dir();

//    if (projectDir.isEmpty())
//        return QString();

    return "projectDir + '/' + DIR_BUILD";
}

void Build::set(const QString& /*setting*/, const QVariant& /*value*/)
{
//    if (!dir().isEmpty() && mkdir(dir())) {
//        auto propertyPath = dir() + '/' + FILE_NAME;
//        auto propertyData = rdfile(propertyPath);
//        auto obj = QJsonDocument::fromJson(propertyData).object();
//        obj[setting] = QJsonValue::fromVariant(value);
//        propertyData = QJsonDocument(obj).toJson();
//        wrfile(propertyPath, propertyData);
//    }
}

void Build::setModule(const QString& /*module*/, bool /*value*/, bool /*qt*/)
{
//    if (!dir().isEmpty() && mkdir(dir())) {
//        auto propertyPath = dir() + '/' + FILE_NAME;
//        auto propertyData = rdfile(propertyPath);
//        auto obj = QJsonDocument::fromJson(propertyData).object();
//        auto modules = obj[qt ? TAG_QT_MUDULES : TAG_OW_MUDULES].toObject();
//        modules[module] = value;
//        obj[qt ? TAG_QT_MUDULES : TAG_OW_MUDULES] = modules;
//        propertyData = QJsonDocument(obj).toJson();
//        wrfile(propertyPath, propertyData);
//    }
}

void Build::setIcon(const QString& /*iconPath*/)
{
//    if (!dir().isEmpty()) {
//        auto icnDir = dir() + '/';
//        if (rm(icnDir + "l.png") &&  rm(icnDir + "m.png") &&
//            rm(icnDir + "h.png") && !iconPath.isEmpty()) {
//            QImage i(iconPath);
//            i.scaled(QSize(128, 128), Qt::IgnoreAspectRatio,
//              Qt::SmoothTransformation).save(icnDir + "l.png");
//            i.scaled(QSize(192, 192), Qt::IgnoreAspectRatio,
//              Qt::SmoothTransformation).save(icnDir + "m.png");
//            i.scaled(QSize(256, 256), Qt::IgnoreAspectRatio,
//              Qt::SmoothTransformation).save(icnDir + "h.png");
//        }
//    }
}

void Build::setKeystore(const QString& /*keystorePath*/)
{
//    if (!dir().isEmpty()) {
//        auto ksDir = dir() + '/' + KEYSTORE_NAME;
//        if (rm(ksDir) && !keystorePath.isEmpty())
//            QFile::copy(keystorePath, ksDir);
//    }
}

void Build::clearPermissions()
{
//    if (!dir().isEmpty()) {
//        auto propertyPath = dir() + '/' + FILE_NAME;
//        auto propertyData = rdfile(propertyPath);
//        auto obj = QJsonDocument::fromJson(propertyData).object();
//        obj[TAG_PERMISSIONS] = QJsonArray();
//        propertyData = QJsonDocument(obj).toJson();
//        wrfile(propertyPath, propertyData);
//    }
}

void Build::addPermission(const QString& /*permission*/)
{
//    if (!dir().isEmpty() && mkdir(dir())) {
//        auto propertyPath = dir() + '/' + FILE_NAME;
//        auto propertyData = rdfile(propertyPath);
//        auto obj = QJsonDocument::fromJson(propertyData).object();
//        auto permissions = obj[TAG_PERMISSIONS].toArray();
//        permissions << permission;
//        obj[TAG_PERMISSIONS] = permissions;
//        propertyData = QJsonDocument(obj).toJson();
//        wrfile(propertyPath, propertyData);
//    }
}
