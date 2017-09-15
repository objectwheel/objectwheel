#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>

#define INF_PROJECTNAME "projectName"
#define INF_DESCRIPTION "description"
#define INF_ORGNAME "orgName"
#define INF_ORGIDENT "orgIdent"
#define INF_PROJECT_VERSION "projectVersion"
#define INF_PROJECT_IDENT "projectIdent"
#define INF_OWNER "owner"
#define INF_CRDATE "crDate"
#define INF_MFDATE "mfDate"
#define INF_SIZE "size"

class ProjectManagerPrivate;
class MainWindow;

class ProjectManager : public QObject
{
		Q_OBJECT
		Q_DISABLE_COPY(ProjectManager)

	public:
		explicit ProjectManager(QObject *parent = 0);
        ~ProjectManager();
		static ProjectManager* instance();
        static void setMainWindow(MainWindow* mainWindow);
		static QString projectDirectory(const QString& projectname);
		static bool exists(const QString& projectname);
		static bool buildNewProject(const QString& projectname);
		static bool renameProject(const QString& from, const QString& to);
        static bool exportProject(const QString& projectname, const QString& filepath);
        static bool importProject(const QString& filepath);
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
		static ProjectManagerPrivate* _d;
};

#endif // PROJECTMANAGER_H
