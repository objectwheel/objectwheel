#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

#define SIGN_OWDB "T3dkYl92Mi4w"
#define SIGN_OWCTRL "T3djdHJsX3YyLjA"
#define DIR_THIS "t"
#define DIR_CHILDREN "c"
#define DIR_OWDB "owdb"
#define DIR_MAINFORM "1"
#define DIR_QRC_OWDB ":/resources/qmls/owdb"
#define FILE_PROPERTIES "_properties.json"
#define TAG_ID "id"
#define TAG_UID "_uid"
#define TAG_SUID "_suid"
#define TAG_OWDB_SIGN "_owdbsign"
#define TAG_OWCTRL_SIGN "_owctrlsign"

class SaveManagerPrivate;
class Control;
class Form;

class SaveManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(SaveManager)

    public:
        explicit SaveManager(QObject *parent = 0);
        static SaveManager* instance();

        static bool initProject(const QString& projectDirectory);
        static void exposeProject();
        static bool execProject();

        static QString basePath();
        static bool isOwctrl(const QString& rootPath);
        static QString id(const QString& rootPath);
        static QString uid(const QString& rootPath);
        static QString suid(const QString& rootPath);
        static bool exists(const Control* control, const QString& suid = QString());
        static bool addForm(Form* form);
        static void removeForm(const Form* form);
        static bool addControl(Control* control, const Control* parentControl, const QString& suid);
        static bool moveControl(Control* control, const Control* parentControl);
        static void removeControl(const Control* control);

        static void setProperty(Control* control, const QString& property, const QVariant& value);
        static void removeProperty(const Control* control, const QString& property);

        static QString pathOfId(const QString& suid, const QString& id, const QString& rootPath = QString());

      static bool inprogress();

    signals:
        void databaseChanged();

    private:
        static SaveManagerPrivate* _d;
};

#endif // SAVEMANAGER_H
