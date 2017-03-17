#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

class SaveManagerPrivate;

class SaveManager : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(SaveManager)

	public:
		explicit SaveManager(QObject *parent = 0);
		static SaveManager* instance();
		static QString saveDirectory(const QString& id);
		static bool exists(const QString& id);
		static void addSave(const QString& id, const QString& url);
		static void removeSave(const QString& id);
		static QStringList saves();
		static void setVariantProperty(const QString& id, const QString& property, const QVariant& value);

	private:
		static SaveManagerPrivate* m_d;
};

#endif // SAVEMANAGER_H
