#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

#define FORMS_DIR "forms"
#define EVENTS_FILE "events.json"
#define TARGET_EVENT_TAG "event"
#define EVENT_CODE_TAG "code"

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
                QString methodName;
                QString methodCode;
        };

        explicit SaveManager(QObject *parent = 0);
        static SaveManager* instance();
        ~SaveManager();

        static bool buildNewDatabase(const QString& projDir);
        static bool loadDatabase();

        static bool exists(const Control* control);
        static void addForm(const Form* form);
        static void removeForm(const Form* form);
        static void addControl(const Control* control, const Control* parentControl);
        static void removeControl(const Control* control);

        static void setProperty(const Control* control, const QString& property, const QVariant& value);
        static void removeProperty(const Control* control, const QString& property);

        static void setEvent(const Control* control, const Event& event);
        static void updateEvent(const Control* control, const QString& sign, const Event& event);
        static void removeEvent(const Control* control, const QString& sign);

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

//      static bool inprogress();

    signals:
        void databaseChanged();

    private:
        static SaveManagerPrivate* m_d;
};

#endif // SAVEMANAGER_H
