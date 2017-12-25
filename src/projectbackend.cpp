#include <projectbackend.h>
#include <userbackend.h>
#include <filemanager.h>
#include <toolsbackend.h>
#include <savebackend.h>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <mainwindow.h>
#include <zipper.h>
#include <designerwidget.h>

#define INF_FILENAME "inf.json"

// Returns biggest number from integer named dirs.
// If no integer named dir exists, 0 returned.
// If no dir exists or dirs are smaller than zero, 0 returned.
int biggestDir(const QString& basePath)
{
    int num = 0;
    for (auto dir : lsdir(basePath))
        if (dir.toInt() > num)
            num = dir.toInt();
    return num;
}

static QString generateProjectDir(const QString& projectname)
{
	auto userDir = UserBackend::userDirectory(UserBackend::currentSessionsUser());
    if (userDir.isEmpty()) return userDir;
	return userDir + separator() + QByteArray().insert(0, projectname).toHex();
}

static QString bytesString(const qint64 size)
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

ProjectBackend* ProjectBackend::instance()
{
    static ProjectBackend instance;
    return &instance;
}

QString ProjectBackend::projectDirectory(const QString& projectname)
{
    if (!exists(projectname)) return QString();
    return generateProjectDir(projectname);
}

bool ProjectBackend::exists(const QString& projectname)
{
    auto projectDir = generateProjectDir(projectname);
	if (projectDir.isEmpty()) return false;
	return ::exists(projectDir);
}

bool ProjectBackend::buildNewProject(const QString& projectname)
{
    return (!UserBackend::currentSessionsUser().isEmpty() &&
            !exists(projectname) &&
            mkdir(generateProjectDir(projectname)) &&
            SaveBackend::initProject(generateProjectDir(projectname)));
}

bool ProjectBackend::renameProject(const QString& from, const QString& to)
{
	if (!exists(from) || exists(to)) return false;
    auto fromDir = generateProjectDir(from);
    auto toDir = generateProjectDir(to);
	if (fromDir.isEmpty() || toDir.isEmpty()) return false;
    if (_currentProject == from) {
        stopProject();
//        mainWindow->clearStudio(); //FIXME
        if (!rn(fromDir, toDir)) return false;
        if (!startProject(to)) return false;
		return infUpdateLastModification();
	} else {
		return rn(fromDir, toDir);
    }
}

bool ProjectBackend::exportProject(const QString& projectname, const QString& filepath)
{
    if (!exists(projectname)) return false;
    auto projDir = generateProjectDir(projectname);
    return Zipper::compressDir(projDir, filepath, fname(projDir));
}

bool ProjectBackend::importProject(const QString &filepath)
{
    auto zipData = rdfile(filepath);
    if (zipData.isEmpty()) return false;
    auto userDir = UserBackend::userDirectory(UserBackend::currentSessionsUser());
    if (userDir.isEmpty()) return false;
    return Zipper::extractZip(zipData, userDir);
}

bool ProjectBackend::fillProjectInformation(
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
    )
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

QJsonObject ProjectBackend::projectInformation(const QString& projectname)
{
	auto projDir = projectDirectory(projectname);
	if (projDir.isEmpty()) return QJsonObject();
	if (!::exists(projDir + separator() + INF_FILENAME)) return QJsonObject();
	return QJsonDocument::fromJson(rdfile(projDir + separator() + INF_FILENAME)).object();
}

bool ProjectBackend::infUpdateSize()
{
    if (_currentProject.isEmpty()) return false;
    auto jObj = projectInformation(_currentProject);
	if (jObj.isEmpty()) return false;
    auto projDir = projectDirectory(_currentProject);
	if (projDir.isEmpty()) return false;
    jObj[INF_SIZE] = bytesString(dsize(projDir));
	QJsonDocument jDoc(jObj);
	if (wrfile(projDir + separator() + INF_FILENAME, jDoc.toJson()) < 0) return false;
	else return true;
}

bool ProjectBackend::infUpdateLastModification()
{
    if (_currentProject.isEmpty()) return false;
    auto jObj = projectInformation(_currentProject);
	if (jObj.isEmpty()) return false;
    auto projDir = projectDirectory(_currentProject);
	if (projDir.isEmpty()) return false;
	jObj[INF_MFDATE] = QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ");
	QJsonDocument jDoc(jObj);
	if (wrfile(projDir + separator() + INF_FILENAME, jDoc.toJson()) < 0) return false;
	else return true;
}

bool ProjectBackend::startProject(const QString& projectname)
{
	if (UserBackend::currentSessionsUser().isEmpty()) return false;

    if (_currentProject == projectname) {
		return true;
	}

	if (!exists(projectname)) {
		return false;
	}

    if (!_currentProject.isEmpty()) {
        stopProject();
    }

//    mainWindow->clearStudio(); //FIXME

    _currentProject = projectname;

    SaveBackend::exposeProject();
    DesignerWidget::controlScene()->clearSelection();
    DesignerWidget::formScene()->clearSelection();

    ToolsBackend::instance()->downloadTools();

//    MainWindow::instance()->setWindowTitle
//      (QString(APP_NAME) + " - [" + projectname + "]"); //FIXME

	return true;
}

void ProjectBackend::stopProject()
{
	if (UserBackend::currentSessionsUser().isEmpty()) return;

    if (_currentProject.isEmpty()) {
		return;
	}

	infUpdateSize();
	infUpdateLastModification();

    _currentProject = "";
}

QString ProjectBackend::currentProject()
{
    return _currentProject;
}

QStringList ProjectBackend::projects()
{
	QStringList projectList;
	auto userDir = UserBackend::userDirectory(UserBackend::currentSessionsUser());
	if (userDir.isEmpty()) return projectList;
	for (auto projectname : lsdir(userDir)) {
		projectList << QByteArray::fromHex(QByteArray().insert(0, projectname));
	}
	return projectList;
}
