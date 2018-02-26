#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QObject>
#include <QStringList>
#include <QJsonValue>

#define SIGN_OWDB       "T3dkYl92Mi4w"
#define SIGN_OWCTRL     "T3djdHJsX3YyLjA"
#define DIR_THIS        "t"
#define DIR_CHILDREN    "c"
#define DIR_OWDB        "owdb"
#define DIR_MAINFORM    "1"
#define DIR_QRC_OWDB    ":/resources/qmls/owdb"
#define DIR_QRC_FORM    ":/resources/qmls/form"
#define FILE_PROPERTIES "_properties.json"
#define FILE_ICON       "icon.png" //TODO: Apply everywhere
#define FILE_MAIN       "main.qml" //TODO: Apply everywhere
#define TAG_ID          "id"
#define TAG_X           "x"
#define TAG_Y           "y"
#define TAG_Z           "z"
#define TAG_WIDTH       "width"
#define TAG_HEIGHT      "height"
#define TAG_UID         "_uid"
#define TAG_SUID        "_suid"
#define TAG_SKIN        "_skin"
#define TAG_NAME        "_name"
#define TAG_CATEGORY    "_category"
#define TAG_OWDB_SIGN   "_owdbsign"
#define TAG_OWCTRL_SIGN "_owctrlsign"

class SaveUtils : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(SaveUtils)

    public:
        enum Skin {
            Invalid,
            PhonePortrait,
            PhoneLandscape,
            Desktop,
            NoSkin
        };
        Q_ENUM(Skin)

    public:
        static int biggestDir(const QString& basePath);
        static QStringList formPaths(const QString& projectDir);
        static QStringList controlPaths(const QString& topPath);
        static QStringList childrenPaths(const QString& rootPath, QString suid = QString());
        static QStringList masterPaths(const QString& topPath);
        static bool isOwctrl(const QByteArray& propertyData);
        static bool isOwctrl(const QString& rootPath);
        static bool isMain(const QString& rootPath);
        static Skin skin(const QString& rootPath);
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
        static QJsonValue property(const QByteArray& propertyData, const QString& property);
        static void setProperty(QByteArray& propertyData, const QString& property, const QJsonValue& value);
        static void refreshToolUid(const QString& toolRootPath);
        static void flushId(const QString& topPath, const QString& id);
        static void flushSuid(const QString& topPath, const QString& suid);
        static void updateFile(const QString& filePath, const QString& from, const QString& to);

    private:
        SaveUtils() {}
};

#endif // SAVEUTILS_H
