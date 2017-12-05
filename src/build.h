#ifndef BUILD_H
#define BUILD_H

#include <QString>
#include <QVariant>

#define TAG_TARGET "target"
#define TAG_OW_MUDULES "owmodules"
#define TAG_QT_MUDULES "qtmodules"
#define TAG_APPNAME "appname"
#define TAG_VERSIONNAME "versionname"
#define TAG_VERSIONCODE "versioncode"
#define TAG_ORIENTATION "orientation"
#define TAG_PACKAGENAME "packagename"
#define TAG_MINAPI "minapi"
#define TAG_TARGETAPI "targetapi"
#define TAG_PERMISSIONS "permissions"
#define TAG_KSPW "kspw"
#define TAG_ALIASNAME "aliasname"
#define TAG_ALIASPW "aliaspw"
#define TAG_SIGNON "signon"

class Build
{
        Q_DISABLE_COPY(Build)

    public:
        static QString dir();
        static void set(const QString& setting, const QVariant& value);
        static void setModule(const QString& module, bool value, bool qt);
        static void setIcon(const QString& iconPath);
        static void setKeystore(const QString& keystorePath);

        static void clearPermissions();
        static void addPermission(const QString& permission);

    private:
        Build() {}
};

#endif // BUILD_H
