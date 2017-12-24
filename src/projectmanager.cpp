#include <projectmanager.h>
#include <usermanager.h>
#include <filemanager.h>
#include <toolsmanager.h>
#include <savebackend.h>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <mainwindow.h>
#include <zipper.h>
#include <designerwidget.h>

#define INF_FILENAME "inf.json"

class ProjectManagerPrivate
{
	public:
		ProjectManagerPrivate(ProjectManager* uparent);
		QString generateProjectDir(const QString& projectname) const;
		QString bytesString(const qint64 size);

	public:
		ProjectManager* parent = nullptr;
		QString currentProject;
        MainWindow* mainWindow;
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

QString ProjectManagerPrivate::bytesString(const qint64 size)
{
	QString ret;
	float kb = 1024.0f;
	float mb = 1048576.0f;
	float gb = 1073741824.0f;

	if (size < kb) {
		ret = QString::number(size);
		ret += " Bytes";
	} else if (size < mb) {
		ret = QString::number(size / kb, 'f', 1);
		ret += " KB";
	} else if (size < gb) {
		ret = QString::number(size / mb, 'f', 1);
		ret += " MB";
	} else {
		ret = QString::number(size / gb, 'f', 2);
		ret += " GB";
	}

	return ret;
}

ProjectManagerPrivate* ProjectManager::_d = nullptr;

ProjectManager::ProjectManager(QObject *parent)
	: QObject(parent)
{
	if (_d) return;
    _d = new ProjectManagerPrivate(this);
}

ProjectManager::~ProjectManager()
{
    delete _d;
}

ProjectManager* ProjectManager::instance()
{
	return _d->parent;
}

void ProjectManager::setMainWindow(MainWindow* mainWindow)
{
	_d->mainWindow = mainWindow;
}

QString ProjectManager::projectDirectory(const QString& projectname)
{
    if (!exists(projectname)) return QString();
    return _d->generateProjectDir(projectname);
}

bool ProjectManager::exists(const QString& projectname)
{
	auto projectDir = _d->generateProjectDir(projectname);
	if (projectDir.isEmpty()) return false;
	return ::exists(projectDir);
}

bool ProjectManager::buildNewProject(const QString& projectname)
{
    return (!UserManager::currentSessionsUser().isEmpty() &&
            !exists(projectname) &&
            mkdir(_d->generateProjectDir(projectname)) &&
            SaveBackend::initProject(_d->generateProjectDir(projectname)));
}

bool ProjectManager::renameProject(const QString& from, const QString& to)
{
	if (!exists(from) || exists(to)) return false;
	auto fromDir = _d->generateProjectDir(from);
	auto toDir = _d->generateProjectDir(to);
	if (fromDir.isEmpty() || toDir.isEmpty()) return false;
    if (_d->currentProject == from) {
        stopProject();
//        _d->mainWindow->clearStudio(); //FIXME
        if (!rn(fromDir, toDir)) return false;
        if (!startProject(to)) return false;
		return infUpdateLastModification();
	} else {
		return rn(fromDir, toDir);
    }
}

bool ProjectManager::exportProject(const QString& projectname, const QString& filepath)
{
    if (!exists(projectname)) return false;
    auto projDir = _d->generateProjectDir(projectname);
    return Zipper::compressDir(projDir, filepath, fname(projDir));
}

bool ProjectManager::importProject(const QString &filepath)
{
    auto zipData = rdfile(filepath);
    if (zipData.isEmpty()) return false;
    auto userDir = UserManager::userDirectory(UserManager::currentSessionsUser());
    if (userDir.isEmpty()) return false;
    return Zipper::extractZip(zipData, userDir);
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
	if (_d->currentProject.isEmpty()) return false;
	auto jObj = projectInformation(_d->currentProject);
	if (jObj.isEmpty()) return false;
	auto projDir = projectDirectory(_d->currentProject);
	if (projDir.isEmpty()) return false;
	jObj[INF_SIZE] = _d->bytesString(dsize(projDir));
	QJsonDocument jDoc(jObj);
	if (wrfile(projDir + separator() + INF_FILENAME, jDoc.toJson()) < 0) return false;
	else return true;
}

bool ProjectManager::infUpdateLastModification()
{
	if (_d->currentProject.isEmpty()) return false;
	auto jObj = projectInformation(_d->currentProject);
	if (jObj.isEmpty()) return false;
	auto projDir = projectDirectory(_d->currentProject);
	if (projDir.isEmpty()) return false;
	jObj[INF_MFDATE] = QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ");
	QJsonDocument jDoc(jObj);
	if (wrfile(projDir + separator() + INF_FILENAME, jDoc.toJson()) < 0) return false;
	else return true;
}

bool ProjectManager::startProject(const QString& projectname)
{
	if (UserManager::currentSessionsUser().isEmpty()) return false;

	if (_d->currentProject == projectname) {
		return true;
	}

	if (!exists(projectname)) {
		return false;
	}

    if (!_d->currentProject.isEmpty()) {
        stopProject();
    }

//    _d->mainWindow->clearStudio(); //FIXME

    _d->currentProject = projectname;

    SaveBackend::exposeProject();
    DesignerWidget::controlScene()->clearSelection();
    DesignerWidget::formScene()->clearSelection();

    ToolsManager::instance()->downloadTools();

//    MainWindow::instance()->setWindowTitle
//      (QString(APP_NAME) + " - [" + projectname + "]"); //FIXME

	return true;
}

void ProjectManager::stopProject()
{
	if (UserManager::currentSessionsUser().isEmpty()) return;

	if (_d->currentProject.isEmpty()) {
		return;
	}

	infUpdateSize();
	infUpdateLastModification();

	_d->currentProject = "";
}

QString ProjectManager::currentProject()
{
	return _d->currentProject;
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
