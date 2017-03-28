#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>

class ProjectManagerPrivate;

class ProjectManager : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(ProjectManager)

	public:
		explicit ProjectManager(QObject *parent = 0);
		static ProjectManager* instance();
		static QString projectDirectory(const QString& projectname);
		static bool exists(const QString& projectname);
		static bool buildNewProject(const QString& projectname);
		static bool fillProjectInformation(const QString& projectname,
										   const QString& description,
										   const QString& orgname,
										   const QString& orgIdent,
										   const QString& projectVersion,
										   const QString& projectIdent,
										   const QString& owner,
										   const QString& crDate,
										   const QString& mfDate,
										   const QString& size);
		static QJsonObject projectInformation(const QString& projectname);
		static bool infUpdateSize();
		static bool infUpdateLastModification();
		static bool startProject(const QString& projectname);
		static void stopProject();
		static QString currentProject();
		static QStringList projects();

	private:
		static ProjectManagerPrivate* m_d;
};

#endif // PROJECTMANAGER_H
