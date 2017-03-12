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
		static bool startProject(const QString& projectname);
		static void stopProject();
		static QString currentProject();
		static QStringList projects();

	private:
		static ProjectManagerPrivate* m_d;
};

#endif // PROJECTMANAGER_H
