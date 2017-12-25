#ifndef PROJECTBACKEND_H
#define PROJECTBACKEND_H

#include <QObject>

#define INF_PROJECTNAME     "projectName"
#define INF_DESCRIPTION     "description"
#define INF_ORGNAME         "orgName"
#define INF_ORGIDENT        "orgIdent"
#define INF_PROJECT_VERSION "projectVersion"
#define INF_PROJECT_IDENT   "projectIdent"
#define INF_OWNER           "owner"
#define INF_CRDATE          "crDate"
#define INF_MFDATE          "mfDate"
#define INF_SIZE            "size"

class ProjectBackend : public QObject
{
		Q_OBJECT
        Q_DISABLE_COPY(ProjectBackend)

	public:
        static ProjectBackend* instance();
        QString projectDirectory(const QString& projectname);
        bool exists(const QString& projectname);
        bool buildNewProject(const QString& projectname);
        bool renameProject(const QString& from, const QString& to);
        bool exportProject(const QString& projectname, const QString& filepath);
        bool importProject(const QString& filepath);
        bool fillProjectInformation(
            const QString& projectname,
            const QString& description,
            const QString& orgname,
            const QString& orgIdent,
            const QString& projectVersion,
            const QString& projectIdent,
            const QString& owner,
            const QString& crDate,
            const QString& mfDate,
            const QString& size
        );
        QJsonObject projectInformation(const QString& projectname);
        bool infUpdateSize();
        bool infUpdateLastModification();

        QStringList projects();
        QString currentProject();

    public slots:
        void stopProject();
        bool startProject(const QString& projectname);

	private:
        ProjectBackend() {}
        QString _currentProject;
};

#endif // PROJECTBACKEND_H
