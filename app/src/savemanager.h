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
#define FILE_EVENTS "_events.json"
#define FILE_PROPERTIES "_properties.json"
#define TAG_EVENT_NAME "name"
#define TAG_EVENT_CODE "code"
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
        struct Binding {
                QString sing;
                Control* source;
                QString sourceProperty;
                QString targetProperty;
        };

        struct Event {
                QString sign;
                QString name;
                QString code;
                explicit operator bool() const
                {
                    return (!sign.isEmpty() &&
                            !name.isEmpty() &&
                            !code.isEmpty());
                }
        };

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

        static void setProperty(const Control* control, const QString& property, const QVariant& value);
        static void removeProperty(const Control* control, const QString& property);

        static void setEvent(const Control* control, const Event& event);
        static void setEvent(const QString& rootPath, const Event& event);
        static void updateEvent(const QString& suid, const QString& sign, const Event& event);
        static void removeEvent(const QString& suid, const QString& sign);
        static Event event(const QString& suid, const QString& sign, const QString& rootPath = QString());
        static QString idOfEventOwner(const QString& suid, const QString& sign, const QString& rootPath = QString());
        static QString pathOfId(const QString& suid, const QString& id, const QString& rootPath = QString());

//        static void setBinding(const Control* control, const Binding& binding);
//        static void updateBinding(const Control* control, const QString& sign, const Binding& binding);
//        static void removeBinding(const Control* control, const QString& sign);

//		static QStringList saves();
//      static QStringList childSaves(const QString& id);
//      static void removeChildSavesOnly(const QString& id);
//		static QString saveDirectory(const QString& id);
//      static QString savesDirectory();

//		static QJsonObject getBindingSaves();
//      static QJsonObject getEventSaves();
//		static QJsonObject getParentalRelationships();
//      static QJsonArray getFormOrders();

      static bool inprogress();

    signals:
        void databaseChanged();

    private:
        static SaveManagerPrivate* _d;
};

#endif // SAVEMANAGER_H
