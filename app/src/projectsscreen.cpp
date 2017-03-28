#include <projectsscreen.h>
#include <fit.h>
#include <projectmanager.h>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <usermanager.h>
#include <QDateTime>
#include <QTimer>
#include <splashscreen.h>
#include <scenemanager.h>

QQuickItem* swipeView;
QQuickItem* projectsPage;
QQuickItem* projectSettings;
QQuickItem* projectList;
QQuickItem* projectButton;
QQuickItem* warning;
QQuickItem* projectnameTextInput;
QQuickItem* descriptionTextInput;
QQuickItem* orgnameTextInput;
QQuickItem* orgIdentTextInput;
QQuickItem* projectVersionTextInput;
QQuickItem* projectIdentText;
QQuickItem* ownerText;
QQuickItem* crDateText;
QQuickItem* mfDateText;
QQuickItem* sizeText;
QQuickItem* btnDelete;
QQuickItem* btnImport;
QQuickItem* btnExport;
QQuickItem* btnOk;
QQuickItem* btnCancel;
QQuickItem* listView;
ProjectListModel model;

ProjectsScreen::ProjectsScreen(QWidget *parent)
	: QQuickWidget(parent)
{
	rootContext()->setContextProperty("dpi", Fit::ratio());
	setSource(QUrl("qrc:/resources/qmls/projectsScreen/main.qml"));
	setResizeMode(SizeRootObjectToView);

	swipeView = (QQuickItem*)QQmlProperty::read(rootObject(), "swipeView", engine()).value<QObject*>();
	projectsPage = (QQuickItem*)QQmlProperty::read(rootObject(), "projectsPage", engine()).value<QObject*>();
	projectSettings = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings", engine()).value<QObject*>();
	projectList = (QQuickItem*)QQmlProperty::read(rootObject(), "projectsPage.projectList", engine()).value<QObject*>();
	projectButton = (QQuickItem*)QQmlProperty::read(rootObject(), "projectsPage.projectButton", engine()).value<QObject*>();
	warning = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.warning", engine()).value<QObject*>();
	projectnameTextInput = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.projectnameTextInput", engine()).value<QObject*>();
	descriptionTextInput = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.descriptionTextInput", engine()).value<QObject*>();
	orgnameTextInput = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.orgnameTextInput", engine()).value<QObject*>();
	orgIdentTextInput = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.orgIdentTextInput", engine()).value<QObject*>();
	projectVersionTextInput = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.projectVersionTextInput", engine()).value<QObject*>();
	projectIdentText = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.projectIdentText", engine()).value<QObject*>();
	ownerText = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.ownerText", engine()).value<QObject*>();
	crDateText = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.crDateText", engine()).value<QObject*>();
	mfDateText = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.mfDateText", engine()).value<QObject*>();
	sizeText = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.sizeText", engine()).value<QObject*>();
	btnDelete = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.btnDelete", engine()).value<QObject*>();
	btnImport = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.btnImport", engine()).value<QObject*>();
	btnExport = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.btnExport", engine()).value<QObject*>();
	btnOk = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.btnOk", engine()).value<QObject*>();
	btnCancel = (QQuickItem*)QQmlProperty::read(rootObject(), "projectSettings.btnCancel", engine()).value<QObject*>();
	listView = (QQuickItem*)QQmlProperty::read(projectList, "listView", engine()).value<QObject*>();

	connect(projectButton, SIGNAL(newButtonClicked()), this, SLOT(handleNewButtonClicked()));
	connect(projectButton, SIGNAL(loadButtonClicked()), this, SLOT(handleLoadButtonClicked()));
	connect(projectList, SIGNAL(infoClicked(QVariant)), this, SLOT(handleInfoButtonClicks(QVariant)));

	QVariant v;
	v.setValue<ProjectListModel*>(&model);
	listView->setProperty("model", v);
}

void ProjectsScreen::handleNewButtonClicked()
{
	if (UserManager::userDirectory(UserManager::currentSessionsUser()).isEmpty()) return;
	auto projects = ProjectManager::projects();
	int count = 1;
	QString projectName = "New Project-1";
	while (projects.contains(projectName)) {
		count++;
		projectName.remove(projectName.size() - 1, 1);
		projectName += QString::number(count);
	}

	int lastIndex = model.rowCount();
	model.insertRow(lastIndex);
	listView->setProperty("currentIndex", lastIndex);
	model.set(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole], projectName);
	model.set(lastIndex, model.roleNames()[ProjectListModel::ActiveRole], false);
	model.set(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole],
			QDateTime::currentDateTime().toString(Qt::ISODate));

	sizeText->setProperty("text", "0 bytes");
	mfDateText->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole]));
	crDateText->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole]));
	ownerText->setProperty("text", UserManager::currentSessionsUser());
	projectVersionTextInput->setProperty("text", "v0.01");
	orgIdentTextInput->setProperty("text", "com.example");
	orgnameTextInput->setProperty("text", "Example Corp.");
	descriptionTextInput->setProperty("text", "Simple description here.");
	projectnameTextInput->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole]));
	QTimer::singleShot(800, [=]{ swipeView->setProperty("currentIndex", 1); });
}

void ProjectsScreen::handleInfoButtonClicks(const QVariant& projectname)
{
	auto projectName = projectname.toString();
	sizeText->setProperty("text", ProjectManager::projectInformation(projectName)[INF_SIZE].toString());
	mfDateText->setProperty("text", ProjectManager::projectInformation(projectName)[INF_MFDATE].toString());
	crDateText->setProperty("text", ProjectManager::projectInformation(projectName)[INF_CRDATE].toString());
	ownerText->setProperty("text", ProjectManager::projectInformation(projectName)[INF_OWNER].toString());
	projectVersionTextInput->setProperty("text", ProjectManager::projectInformation(projectName)[INF_PROJECT_VERSION].toString());
	orgIdentTextInput->setProperty("text", ProjectManager::projectInformation(projectName)[INF_ORGIDENT].toString());
	orgnameTextInput->setProperty("text", ProjectManager::projectInformation(projectName)[INF_ORGNAME].toString());
	descriptionTextInput->setProperty("text", ProjectManager::projectInformation(projectName)[INF_DESCRIPTION].toString());
	projectnameTextInput->setProperty("text", ProjectManager::projectInformation(projectName)[INF_PROJECTNAME].toString());
	swipeView->setProperty("currentIndex", 1);
}

void ProjectsScreen::handleLoadButtonClicked()
{
	auto projectName = model.get(listView->property("currentIndex").toInt(),
								 model.roleNames()[ProjectListModel::ProjectNameRole]).toString();
	auto currentProject = ProjectManager::currentProject();
	auto count = listView->property("count").toInt();
	if (!currentProject.isEmpty()) {
		for (;count--;) {
			if (model.get(count - 1, model.roleNames()[ProjectListModel::ActiveRole]).toBool() &&
				currentProject == projectName) {
				SceneManager::show("studioScene", SceneManager::ToLeft);
				return;
			}
		}
	}
	SplashScreen::show(3000); //FIXME:
	ProjectManager::stopProject();
	ProjectManager::startProject(projectName);
	QTimer::singleShot(4000, [=] { SceneManager::show("studioScene", SceneManager::ToLeft); });
}

void ProjectsScreen::refreshProjectList(const QString& activeProject)
{
	model.clear();
	if (UserManager::userDirectory(UserManager::currentSessionsUser()).isEmpty()) return;
	auto projects = ProjectManager::projects();
	if (projects.size() < 1) return;
	for (auto project : projects) {
		int lastIndex = model.rowCount();
		model.insertRow(lastIndex);
		model.set(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole], project);
		model.set(lastIndex, model.roleNames()[ProjectListModel::ActiveRole], project == activeProject);
		model.set(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole],
				ProjectManager::projectInformation(project)[INF_MFDATE].toString());
	}
	listView->setProperty("currentIndex", 0);
}
