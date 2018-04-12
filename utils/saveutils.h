#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <utils_global.h>
#include <QObject>
#include <QStringList>
#include <QJsonValue>

#define SIGN_OWDB        "T3dkYl92Mi4w"
#define SIGN_OWCTRL      "T3djdHJsX3YyLjA"
#define SIGN_OWPRJ       "T3dwcmpfdjIuMA"
#define DIR_THIS         "t"
#define DIR_CHILDREN     "c"
#define DIR_OWDB         "owdb"
#define DIR_MAINFORM     "1"
#define FILE_PROJECT     "project.json"
#define FILE_PROPERTIES  "properties.json"
#define FILE_ICON        "icon.png" //TODO: Apply everywhere
#define FILE_MAIN        "main.qml" //TODO: Apply everywhere
#define TAG_X            "x"
#define TAG_Y            "y"
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
#define PTAG_OWPRJ_SIGN  "owprj"

class UTILS_EXPORT SaveUtils : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(SaveUtils)

    public:
        static int biggestDir(const QString& basePath);
        static QStringList formPaths(const QString& projectDir);
        static QStringList controlPaths(const QString& topPath);
        static QStringList childrenPaths(const QString& rootPath, QString suid = QString());
        static QStringList masterPaths(const QString& topPath);
        static bool isOwctrl(const QByteArray& propertyData);
        static bool isOwctrl(const QString& rootPath);
        static bool isForm(const QString& rootPath);
        static bool isMain(const QString& rootPath);
        static qreal x(const QString& rootPath);
        static qreal y(const QString& rootPath);
        static QString id(const QString& rootPath);
        static QString uid(const QString& rootPath);
        static QString suid(const QString& rootPath);
        static QString toolName(const QString& toolRootPath);
        static QString toolCategory(const QString& toolRootPath);
        static QJsonValue property(const QByteArray& propertyData, const QString& property);
        static void setX(const QString& rootPath, qreal x);
        static void setY(const QString& rootPath, qreal y);
        static void setProperty(QByteArray& propertyData, const QString& property, const QJsonValue& value);
        static void refreshToolUid(const QString& toolRootPath);
        static void flushId(const QString& topPath, const QString& id);
        static void flushSuid(const QString& topPath, const QString& suid);
        static void updateFile(const QString& filePath, const QString& from, const QString& to);

        static bool isOwprj(const QByteArray& propertyData);
        static bool isOwprj(const QString& projectDir);
        static QString hash(const QString& projectDir);
        static QString projectName(const QString& projectDir);
        static QString description(const QString& projectDir);
        static QString owner(const QString& projectDir);
        static QString crDate(const QString& projectDir);
        static QString mfDate(const QString& projectDir);
        static QString size(const QString& projectDir);
        static QJsonValue theme(const QString& projectDir);
        static void setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value);

    private:
        SaveUtils() {}
};

#endif // SAVEUTILS_H
