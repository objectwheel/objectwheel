#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

#define SAVE_DIRECTORY "dashboard"
#define PARENTAL_RELATIONSHIP_FILE "parental_relationship.json"
#define FORM_ORDER_FILE "form_order.json"
#define BINDINGS_FILE "bindings.json"
#define BINDING_SOURCE_ID_LABEL "sourceId"
#define BINDING_SOURCE_PROPERTY_LABEL "sourceProperty"
#define BINDING_TARGET_ID_LABEL "targetId"
#define BINDING_TARGET_PROPERTY_LABEL "targetProperty"

#define EVENTS_FILE "events.json"
#define EVENT_TARGET_ID_LABEL "targetId"
#define EVENT_TARGET_EVENTNAME_LABEL "targetEventname"
#define EVENT_EVENT_CODE_LABEL "eventCode"

class SaveManagerPrivate;

class SaveManager : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(SaveManager)

	public:
		struct BindingInf {
				QString sourceId;
				QString sourceProperty;
				QString targetId;
				QString targetProperty;
				QString bindingName;
		};

        struct EventInf {
                QString targetId;
                QString targetEventname;
                QString eventCode;
                QString eventName;
        };

		explicit SaveManager(QObject *parent = 0);
        ~SaveManager();
		static SaveManager* instance();

		static bool buildNewDatabase(const QString& projDir);
		static bool loadDatabase();

		static bool exists(const QString& id);
		static QStringList saves();
        static QStringList childSaves(const QString& id);
		static void addSave(const QString& id, const QString& url);
		static void changeSave(const QString& fromId, QString toId);
		static void removeSave(const QString& id);
        static void removeChildSavesOnly(const QString& id);
		static QString saveDirectory(const QString& id);
        static QString savesDirectory();

		static QJsonObject getBindingSaves();
		static void addBindingSave(const BindingInf& bindingInf);
		static void changeBindingSave(const QString& bindingName, const BindingInf& toBindingInf);
		static void removeBindingSave(const QString& bindingName);

        static QJsonObject getEventSaves();
        static void addEventSave(const EventInf& eventInf);
        static void changeEventSave(const QString& eventName, const EventInf& toEventInf);
        static void removeEventSave(const QString& eventName);

		static QJsonObject getParentalRelationships();
        static QJsonArray getFormOrders();

		static void addParentalRelationship(const QString& id, const QString& parent);
		static void removeParentalRelationship(const QString& id);
		static QString parentalRelationship(const QString& id);

        static void addFormOrder(const QString& formId);
        static void removeFormOrder(const QString& formId);
        static void changeFormOrder(const QString& fromFormId, const QString& toFormId);

        static void setVariantProperty(const QString& id, const QString& property, const QVariant& value);
        static void removeVariantProperty(const QString& id, const QString& property);

        static bool inprogress();

    private:
		static SaveManagerPrivate* m_d;
};

#endif // SAVEMANAGER_H
