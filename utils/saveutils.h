#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <utils_global.h>

#include <QJsonValue>

#define SIGN_OWDB        "T3dkYl92Mi4w"
#define SIGN_OWCTRL      "T3djdHJsX3YyLjA"
#define SIGN_OWPRJ       "T3dwcmpfdjIuMA"

#define DIR_THIS         "t"
#define DIR_CHILDREN     "c"
#define DIR_MAINFORM     "1"
#define DIR_OWDB         "owdb"

#define FILE_PROJECT     "project.json"
#define FILE_PROPERTIES  "properties.json"
#define FILE_ICON        "icon.png" //TODO: Apply everywhere
#define FILE_MAIN        "main.qml" //TODO: Apply everywhere

#define TAG_X            "x"
#define TAG_Y            "y"
#define TAG_Z            "z"
#define TAG_WIDTH        "width"
#define TAG_HEIGHT       "height"
#define TAG_ID           "id"
#define TAG_UID          "uid"
#define TAG_SUID         "suid"
#define TAG_NAME         "name"
#define TAG_CATEGORY     "category"
#define TAG_OWDB_SIGN    "owdbsign"
#define TAG_OWCTRL_SIGN  "owctrlsign"

#define PTAG_PROJECTNAME "projectName"
#define PTAG_DESCRIPTION "description"
#define PTAG_OWNER       "owner"
#define PTAG_CRDATE      "crDate"
#define PTAG_MFDATE      "mfDate"
#define PTAG_SIZE        "size"
#define PTAG_HASH        "hash"
#define PTAG_THEME       "theme"
#define PTAG_SCALING     "scaling"
#define PTAG_OWPRJ_SIGN  "owprj"

class UTILS_EXPORT SaveUtils final
{
    Q_DISABLE_COPY(SaveUtils)

public:
    static bool isForm(const QString& rootPath);
    static bool isMain(const QString& rootPath);
    static bool isOwctrl(const QString& rootPath);
    static bool isOwprj(const QString& projectDir);

    static int biggestDir(const QString& basePath);
    static int childrenCount(const QString& rootPath, QString suid = QString());

    static QStringList formPaths(const QString& projectDir);
    static QStringList controlPaths(const QString& topPath);
    static QStringList masterPaths(const QString& topPath);
    static QStringList childrenPaths(const QString& rootPath, QString suid = QString());

    static QString toUrl(const QString& rootPath);
    static QString toParentDir(const QString& topPath);
    static QString toChildrenDir(const QString& rootPath);
    static QString toProjectFile(const QString& projectDir);
    static QString toPropertiesFile(const QString& rootPath);

    static qreal x(const QString& rootPath);
    static qreal y(const QString& rootPath);
    static qreal z(const QString& rootPath);
    static qreal width(const QString& rootPath);
    static qreal height(const QString& rootPath);

    static QString id(const QString& rootPath);
    static QString uid(const QString& rootPath);
    static QString suid(const QString& rootPath);
    static QString toolName(const QString& toolRootPath);
    static QString toolCategory(const QString& toolRootPath);

    static QString projectHash(const QString& projectDir);
    static QString projectName(const QString& projectDir);
    static QString projectSize(const QString& projectDir);
    static QString projectOwner(const QString& projectDir);
    static QString projectCrDate(const QString& projectDir);
    static QString projectMfDate(const QString& projectDir);
    static QString projectScaling(const QString& projectDir);
    static QString projectDescription(const QString& projectDir);
    static QJsonValue projectTheme(const QString& projectDir);

    static QJsonValue property(const QString& rootPath, const QString& property);
    static QJsonValue projectProperty(const QString& projectDir, const QString& property);

    static void setX(const QString& rootPath, qreal x);
    static void setY(const QString& rootPath, qreal y);
    static void setZ(const QString& rootPath, qreal z);
    static void setWidth(const QString& rootPath, qreal width);
    static void setHeight(const QString& rootPath, qreal height);
    static void setId(const QString& rootPath, const QString& id);

    static void setProperty(const QString& rootPath,  const QString& property, const QJsonValue& value);
    static void setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value);

    static void updateFile(const QString& filePath, const QString& from, const QString& to);
    static void recalculateUids(const QString& topPath);

private:
    SaveUtils() {}
};

#endif // SAVEUTILS_H
