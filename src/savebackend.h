#ifndef SAVEBACKEND_H
#define SAVEBACKEND_H

#include <QObject>
#include <global.h>

#define SIGN_OWDB "T3dkYl92Mi4w"
#define SIGN_OWCTRL "T3djdHJsX3YyLjA"
#define DIR_THIS "t"
#define DIR_CHILDREN "c"
#define DIR_OWDB "owdb"
#define DIR_MAINFORM "1"
#define DIR_QRC_OWDB ":/resources/qmls/owdb"
#define DIR_QRC_FORM ":/resources/qmls/form"
#define FILE_PROPERTIES "_properties.json"
#define FILE_ICON "icon.png" //TODO: Apply everywhere
#define FILE_MAIN "main.qml" //TODO: Apply everywhere
#define TAG_ID "id"
#define TAG_X "x"
#define TAG_Y "y"
#define TAG_Z "z"
#define TAG_WIDTH "width"
#define TAG_HEIGHT "height"
#define TAG_UID "_uid"
#define TAG_SUID "_suid"
#define TAG_SKIN "_skin"
#define TAG_NAME "_name"
#define TAG_CATEGORY "_category"
#define TAG_OWDB_SIGN "_owdbsign"
#define TAG_OWCTRL_SIGN "_owctrlsign"

class SaveBackendPrivate;
class Control;
class Form;
class ControlScene;

class SaveBackend : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(SaveBackend)

    public:
        explicit SaveBackend(QObject *parent = 0);
        static SaveBackend* instance();

        static void exposeProject();
        static Control* exposeControl(const QString& rootPath, const DesignMode& mode, QString suid = QString());
        static bool initProject(const QString& projectDirectory);

        static int biggestDir(const QString& basePath);
        static QString basePath();
        static QStringList formPaths();
        static QStringList childrenPaths(const QString& rootPath, QString suid = QString());
        static QStringList masterPaths(const QString& topPath);
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
        static void refreshToolUid(const QString& toolRootPath);
        static QString toolName(const QString& toolRootPath);
        static QString toolCategory(const QString& toolRootPath);

        static bool exists(const Control* control, const QString& suid, const QString& topPath = QString());
        static bool addForm(Form* form);
        static void removeForm(const Form* form);
        static bool addControl(Control* control, const Control* parentControl, const QString& suid, const QString& topPath = QString());
        static bool moveControl(Control* control, const Control* parentControl);
        static void removeControl(const Control* control);
        static void removeChildControlsOnly(const Control* control);

        static void setProperty(Control* control, const QString& property, const QVariant& value, const QString& topPath = QString());
        static void removeProperty(const Control* control, const QString& property);

        static QString pathOfId(const QString& suid, const QString& id, const QString& rootPath = QString());

        static bool parserWorking();

    signals:
        void busyLoader(int progress, const QString& text);
        void doneLoader(const QString& text);
        void busyExecuter(int progress, const QString& text);
        void doneExecuter(const QString& text);
        void parserRunningChanged(bool running);
        void databaseChanged();
        void projectExposed();

    private:
        static SaveBackendPrivate* _d;
};

#endif // SAVEBACKEND_H
