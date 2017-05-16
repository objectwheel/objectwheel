#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

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

		explicit SaveManager(QObject *parent = 0);
        ~SaveManager();
		static SaveManager* instance();

		static bool buildNewDatabase(const QString& projDir);
		static bool loadDatabase();

		static bool exists(const QString& id);
		static QStringList saves();
		static void addSave(const QString& id, const QString& url);
		static void changeSave(const QString& fromId, QString toId);
		static void removeSave(const QString& id);
		static QString saveDirectory(const QString& id);
        static QString savesDirectory();

		static QJsonObject getBindingSaves();
		static void addBindingSave(const BindingInf& bindingInf);
		static void changeBindingSave(const QString& bindingName, const BindingInf& toBindingInf);
		static void removeBindingSave(const QString& bindingName);

		static QJsonObject getParentalRelationships();
		static QJsonArray getPageOrders();

		static void addParentalRelationship(const QString& id, const QString& parent);
		static void removeParentalRelationship(const QString& id);
		static QString parentalRelationship(const QString& id);

		static void addPageOrder(const QString& pageId);
		static void removePageOrder(const QString& pageId);
		static void changePageOrder(const QString& fromPageId, const QString& toPageId);

        static void setId(const QString& id, const QString& newId);
		static void setVariantProperty(const QString& id, const QString& property, const QVariant& value);
		static void setBindingProperty(const QString& id, const QString& property, const QString& expression);
		static void removeProperty(const QString& id, const QString& property);

    public slots:
        void idApplier();

    private:
		static SaveManagerPrivate* m_d;
};

#endif // SAVEMANAGER_H
