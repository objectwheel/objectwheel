#include <projectmanager.h>
#include <usermanager.h>
#include <filemanager.h>
#include <toolsmanager.h>
#include <savemanager.h>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

#define INF_FILENAME "inf.json"
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

class ProjectManagerPrivate
{
	public:
		ProjectManagerPrivate(ProjectManager* uparent);
		QString generateProjectDir(const QString& projectname) const;

	public:
		ProjectManager* parent = nullptr;
		QString currentProject;
};

ProjectManagerPrivate::ProjectManagerPrivate(ProjectManager* uparent)
	: parent(uparent)
{
}

inline QString ProjectManagerPrivate::generateProjectDir(const QString& projectname) const
{
	auto userDir = UserManager::userDirectory(UserManager::currentSessionsUser());
	if (userDir.isEmpty()) return userDir;
	return userDir + separator() + QByteArray().insert(0, projectname).toHex();
}

ProjectManagerPrivate* ProjectManager::m_d = nullptr;

ProjectManager::ProjectManager(QObject *parent)
	: QObject(parent)
{
	if (m_d) return;
	m_d = new ProjectManagerPrivate(this);
}

ProjectManager* ProjectManager::instance()
{
	return m_d->parent;
}

QString ProjectManager::projectDirectory(const QString& projectname)
{
	if (!exists(projectname)) return QString();
	return m_d->generateProjectDir(projectname);
}

bool ProjectManager::exists(const QString& projectname)
{
	auto projectDir = m_d->generateProjectDir(projectname);
	if (projectDir.isEmpty()) return false;
	return ::exists(projectDir);
}

bool ProjectManager::buildNewProject(const QString& projectname)
{
	if (UserManager::currentSessionsUser().isEmpty()) return false;
	if (exists(projectname)) return false;
	if (!mkdir(m_d->generateProjectDir(projectname))) return false;
	if (!SaveManager::buildNewDatabase(m_d->generateProjectDir(projectname))) return false;
	return true;
}

bool ProjectManager::fillProjectInformation(const QString& projectname,
											const QString& description,
											const QString& orgname,
											const QString& orgIdent,
											const QString& projectVersion,
											const QString& projectIdent,
											const QString& owner,
											const QString& crDate,
											const QString& mfDate,
											const QString& size)
{
	auto projDir = projectDirectory(projectname);
	if (projDir.isEmpty()) return false;
	QJsonObject jObj;
	jObj["projectName"] = projectname;
	jObj["description"] = description;
	jObj["orgName"] = orgname;
	jObj["orgIdent"] = orgIdent;
	jObj["projectVersion"] = projectVersion;
	jObj["projectIdent"] = projectIdent;
	jObj["owner"] = owner;
	jObj["crDate"] = crDate;
	jObj["mfDate"] = mfDate;
	jObj["size"] = size;
	QJsonDocument jDoc(jObj);
	if (wrfile(projDir + separator() + INF_FILENAME, jDoc.toJson()) < 0) return false;
	else return true;
}

QJsonObject ProjectManager::projectInformation(const QString& projectname)
{
	auto projDir = projectDirectory(projectname);
	if (projDir.isEmpty()) return QJsonObject();
	if (!::exists(projDir + separator() + INF_FILENAME)) return QJsonObject();
	return QJsonDocument::fromJson(rdfile(projDir + separator() + INF_FILENAME)).object();
}

bool ProjectManager::infUpdateSize()
{
	if (m_d->currentProject.isEmpty()) return false;
	auto jObj = projectInformation(m_d->currentProject);
	if (jObj.isEmpty()) return false;
	auto projDir = projectDirectory(m_d->currentProject);
	if (projDir.isEmpty()) return false;
	jObj[INF_SIZE] = dsize(projDir);
	QJsonDocument jDoc(jObj);
	if (wrfile(projDir + separator() + INF_FILENAME, jDoc.toJson()) < 0) return false;
	else return true;
}

bool ProjectManager::infUpdateLastModification()
{
	if (m_d->currentProject.isEmpty()) return false;
	auto jObj = projectInformation(m_d->currentProject);
	if (jObj.isEmpty()) return false;
	auto projDir = projectDirectory(m_d->currentProject);
	if (projDir.isEmpty()) return false;
	jObj[INF_MFDATE] = QDateTime::currentDateTime().toString(Qt::ISODate);
	QJsonDocument jDoc(jObj);
	if (wrfile(projDir + separator() + INF_FILENAME, jDoc.toJson()) < 0) return false;
	else return true;
}

bool ProjectManager::startProject(const QString& projectname)
{
	if (UserManager::currentSessionsUser().isEmpty()) return false;

	if (m_d->currentProject == projectname) {
		return true;
	}

	if (!exists(projectname)) {
		return false;
	}

	if (!m_d->currentProject.isEmpty()) {
		stopProject();
	}

	m_d->currentProject = projectname;

	if (!SaveManager::loadDatabase()) {
		QMessageBox::warning(NULL, "Oops", "The database is corrupted.");
		return false;
	}

	ToolsManager::downloadTools();

	return true;
}

void ProjectManager::stopProject()
{
	if (UserManager::currentSessionsUser().isEmpty()) return;

	if (m_d->currentProject.isEmpty()) {
		return;
	}

	infUpdateSize();
	infUpdateLastModification();

	/* Clear designer */
//	if (exists(m_d->currentSessionsUser) && !m_d->dirLocker.locked(userDirectory(m_d->currentSessionsUser))) {
//		if (!m_d->dirLocker.lock(userDirectory(m_d->currentSessionsUser), m_d->currentSessionsKey)) qFatal("ProjectManager : Error occurred");
//	}

	m_d->currentProject = "";
}

QString ProjectManager::currentProject()
{
	return m_d->currentProject;
}

QStringList ProjectManager::projects()
{
	QStringList projectList;
	auto userDir = UserManager::userDirectory(UserManager::currentSessionsUser());
	if (userDir.isEmpty()) return projectList;
	for (auto projectname : lsdir(userDir)) {
		projectList << QByteArray::fromHex(QByteArray().insert(0, projectname));
	}
	return projectList;
}
