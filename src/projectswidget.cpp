#include <projectswidget.h>
#include <projectmanager.h>
#include <filemanager.h>
#include <usermanager.h>
#include <fit.h>

#include <QDateTime>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>

ProjectsWidget* instance = nullptr;
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

ProjectsWidget::ProjectsWidget(QWidget *parent)
	: QQuickWidget(parent)
{
    if (::instance) return;
    ::instance = this;
	rootContext()->setContextProperty("dpi", fit::ratio());
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
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(handleBtnCancelClicked()));
	connect(btnDelete, SIGNAL(clicked()), this, SLOT(handleBtnDeleteClicked()));
	connect(btnOk, SIGNAL(clicked()), this, SLOT(handleBtnOkClicked()));
    connect(btnImport, SIGNAL(clicked()), this, SLOT(handleBtnImportClicked()));
    connect(btnExport, SIGNAL(clicked()), this, SLOT(handleBtnExportClicked()));

	QVariant v;
	v.setValue<ProjectListModel*>(&model);
    listView->setProperty("model", v);
}

ProjectsWidget* ProjectsWidget::instance()
{
    return ::instance;
}

void ProjectsWidget::handleNewButtonClicked()
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
			QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " "));

	sizeText->setProperty("text", "0 bytes");
	mfDateText->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole]));
	crDateText->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole]));
	ownerText->setProperty("text", UserManager::currentSessionsUser());
	projectVersionTextInput->setProperty("text", "v0.01");
	orgIdentTextInput->setProperty("text", "com.example");
	orgnameTextInput->setProperty("text", "Example Corp.");
	descriptionTextInput->setProperty("text", "Simple description here.");
	projectnameTextInput->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole]));
	QTimer::singleShot(400, [=]{ swipeView->setProperty("currentIndex", 1); });
}

void ProjectsWidget::handleInfoButtonClicks(const QVariant& projectname)
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

void ProjectsWidget::handleBtnCancelClicked()
{
	refreshProjectList(ProjectManager::currentProject());
	swipeView->setProperty("currentIndex", 0);
}

void ProjectsWidget::handleBtnDeleteClicked()
{
	QString currentProject;
	auto projectName = model.get(listView->property("currentIndex").toInt(),
                                 model.roleNames()[ProjectListModel::ProjectNameRole]).toString();
    if (!ProjectManager::exists(projectName)) goto finish;
    currentProject = ProjectManager::currentProject();
    if (!currentProject.isEmpty() && currentProject == projectName) {
		ProjectManager::stopProject();
		currentProject = "";
    }

	rm(ProjectManager::projectDirectory(projectName));

finish:
	refreshProjectList(currentProject);
    swipeView->setProperty("currentIndex", 0);
}

void ProjectsWidget::handleBtnImportClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Zip files (*.zip)"));
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec()) {
        for (auto fileName : dialog.selectedFiles()) {
            if (!ProjectManager::importProject(fileName)) {
                QMessageBox::warning(this, "Operation Stopped", "One or more projects exists. Please rename your existing projects to another name."
                                     "Or one or more import file corrupted.");
                return;
            }
        }
        refreshProjectList(ProjectManager::currentProject());
        swipeView->setProperty("currentIndex", 0);
        QMessageBox::information(this, "Finished", "Tool import has successfully finished.");
    }
}

void ProjectsWidget::handleBtnExportClicked()
{
    auto projectName = model.get(listView->property("currentIndex").toInt(),
                                 model.roleNames()[ProjectListModel::ProjectNameRole]).toString();
    if (projectName.isEmpty()) return;

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec()) {
        if (!rm(dialog.selectedFiles().at(0) + separator() + projectName + ".zip")) return;
        if (!ProjectManager::exportProject(projectName, dialog.selectedFiles().at(0) + separator() + projectName + ".zip")) return;
        QMessageBox::information(this, "Finished", "Project export has successfully finished.");
    }
}

void ProjectsWidget::startProject()
{
    auto project = model.get(listView->property("currentIndex").toInt(),
      model.roleNames()[ProjectListModel::ProjectNameRole]).toString();

    if (!ProjectManager::startProject(project)) { // Asynchronous Operation
        for (int i = model.rowCount(); i--;) {
            if (model.get(i, model.roleNames()[ProjectListModel::ActiveRole]).toBool()) {
                model.set(i, model.roleNames()[ProjectListModel::ActiveRole], false);
            }
        }
        qFatal("Fatal : ProjectsWidget");
    }

    for (int i = model.rowCount(); i--;) {
        if (model.get(i, model.roleNames()[ProjectListModel::ActiveRole]).toBool()) {
            model.set(i, model.roleNames()[ProjectListModel::ActiveRole], false);
        }
    }
    model.set(listView->property("currentIndex").toInt(),
        model.roleNames()[ProjectListModel::ActiveRole], true);

    emit done();
}

void ProjectsWidget::handleBtnOkClicked()
{
	auto projectnametext = projectnameTextInput->property("text").toString();
	auto descriptiontext = descriptionTextInput->property("text").toString();
	auto orgnametext = orgnameTextInput->property("text").toString();
	auto orgidenttext = orgIdentTextInput->property("text").toString();
	auto projectversiontext = projectVersionTextInput->property("text").toString();
	auto projectidenttext = projectIdentText->property("text").toString();
	auto sizetext = sizeText->property("text").toString();
	auto mfdatetext = QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ");
	auto crdatetext = crDateText->property("text").toString();
	auto ownertext = ownerText->property("text").toString();
	auto prevprojectname = model.get(listView->property("currentIndex").toInt(),
									 model.roleNames()[ProjectListModel::ProjectNameRole]).toString();

	if (projectnametext.isEmpty() || descriptiontext.isEmpty() ||
		orgnametext.isEmpty() || orgidenttext.isEmpty() || projectversiontext.isEmpty()) {
		QMetaObject::invokeMethod(warning, "show");
		return;
	}

	if (!ProjectManager::exists(prevprojectname)) {
		if (!ProjectManager::buildNewProject(projectnametext)) {
            qFatal("ProjectsWidget::handleBtnOkClicked() : Fatal Error. 0x01");
		}
	} else if (prevprojectname != projectnametext) {
		if (ProjectManager::exists(projectnametext)) {
			QMessageBox::warning(NULL, "Oops!", "There is another project with the same name, please change your project name.");
			return;
		}
		if (!ProjectManager::renameProject(prevprojectname, projectnametext)) {
            qFatal("ProjectsWidget::handleBtnOkClicked() : Fatal Error. 0x02");
		}
	}

	if (!ProjectManager::fillProjectInformation(projectnametext, descriptiontext, orgnametext, orgidenttext, projectversiontext,
												projectidenttext, ownertext, crdatetext, mfdatetext, sizetext))
        qFatal("ProjectsWidget::handleBtnOkClicked() : Fatal Error. 0x03");
	refreshProjectList(ProjectManager::currentProject());
	swipeView->setProperty("currentIndex", 0);
}

void ProjectsWidget::handleLoadButtonClicked()
{
	auto projectName = model.get(listView->property("currentIndex").toInt(),
      model.roleNames()[ProjectListModel::ProjectNameRole]).toString();
    auto currentProject = ProjectManager::currentProject();

	if (!currentProject.isEmpty() && currentProject == projectName) {
        emit done();
		return;
    }

    ProjectManager::stopProject();
    QTimer::singleShot(0, this, &ProjectsWidget::startProject);
    emit busy(tr("Loading project"));
}

void ProjectsWidget::refreshProjectList(const QString& activeProject)
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
				"Last edited: " + ProjectManager::projectInformation(project)[INF_MFDATE].toString());
	}
	listView->setProperty("currentIndex", 0);
}
