#include <projectswidget.h>
#include <filemanager.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <fit.h>

#include <QDateTime>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>

QQuickItem* swipeView;
QQuickItem* projectsPage;
QQuickItem* projectSettings;
QQuickItem* projectList;
QQuickItem* projectButton;
QQuickItem* warning;
QQuickItem* projectnameTextInput;
QQuickItem* descriptionTextInput;
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

ProjectsWidget::ProjectsWidget(QWidget *parent) : QQuickWidget(parent)
{
	rootContext()->setContextProperty("dpi", fit::ratio());
	setSource(QUrl("qrc:/resources/qmls/projectsScreen/main.qml"));
	setResizeMode(SizeRootObjectToView);

    swipeView = QQmlProperty::read(rootObject(), "swipeView", engine()).value<QQuickItem*>();
    projectsPage = QQmlProperty::read(rootObject(), "projectsPage", engine()).value<QQuickItem*>();
    projectSettings = QQmlProperty::read(rootObject(), "projectSettings", engine()).value<QQuickItem*>();
    projectList = QQmlProperty::read(rootObject(), "projectsPage.projectList", engine()).value<QQuickItem*>();
    projectButton = QQmlProperty::read(rootObject(), "projectsPage.projectButton", engine()).value<QQuickItem*>();
    warning = QQmlProperty::read(rootObject(), "projectSettings.warning", engine()).value<QQuickItem*>();
    projectnameTextInput = QQmlProperty::read(rootObject(), "projectSettings.projectnameTextInput", engine()).value<QQuickItem*>();
    descriptionTextInput = QQmlProperty::read(rootObject(), "projectSettings.descriptionTextInput", engine()).value<QQuickItem*>();
    ownerText = QQmlProperty::read(rootObject(), "projectSettings.ownerText", engine()).value<QQuickItem*>();
    crDateText = QQmlProperty::read(rootObject(), "projectSettings.crDateText", engine()).value<QQuickItem*>();
    mfDateText = QQmlProperty::read(rootObject(), "projectSettings.mfDateText", engine()).value<QQuickItem*>();
    sizeText = QQmlProperty::read(rootObject(), "projectSettings.sizeText", engine()).value<QQuickItem*>();
    btnDelete = QQmlProperty::read(rootObject(), "projectSettings.btnDelete", engine()).value<QQuickItem*>();
    btnImport = QQmlProperty::read(rootObject(), "projectSettings.btnImport", engine()).value<QQuickItem*>();
    btnExport = QQmlProperty::read(rootObject(), "projectSettings.btnExport", engine()).value<QQuickItem*>();
    btnOk = QQmlProperty::read(rootObject(), "projectSettings.btnOk", engine()).value<QQuickItem*>();
    btnCancel = QQmlProperty::read(rootObject(), "projectSettings.btnCancel", engine()).value<QQuickItem*>();
    listView = QQmlProperty::read(projectList, "listView", engine()).value<QQuickItem*>();

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

void ProjectsWidget::handleNewButtonClicked()
{
    if (UserBackend::instance()->dir().isEmpty()) return;
    auto projects = ProjectBackend::instance()->projects();
	int count = 1;
    QString projectName = "Project - 1";
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
    ownerText->setProperty("text", UserBackend::instance()->user());
    descriptionTextInput->setProperty("text", "Simple description here.");
	projectnameTextInput->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole]));
    QTimer::singleShot(250, [=]{ swipeView->setProperty("currentIndex", 1); });
}

void ProjectsWidget::handleInfoButtonClicks(const QVariant& hash)
{
    auto h = hash.toString();
    ProjectBackend::instance()->updateSize();
    sizeText->setProperty("text", ProjectBackend::instance()->size(h));
    mfDateText->setProperty("text", ProjectBackend::instance()->mfDate(h));
    crDateText->setProperty("text", ProjectBackend::instance()->crDate(h));
    ownerText->setProperty("text", ProjectBackend::instance()->owner(h));
    descriptionTextInput->setProperty("text", ProjectBackend::instance()->description(h));
    projectnameTextInput->setProperty("text", ProjectBackend::instance()->name(h));
	swipeView->setProperty("currentIndex", 1);
}

void ProjectsWidget::handleBtnCancelClicked()
{
    refreshProjectList();
	swipeView->setProperty("currentIndex", 0);
}

void ProjectsWidget::handleBtnDeleteClicked()
{
    QString chash;
    auto hash = model.get(listView->property("currentIndex").toInt(),
      model.roleNames()[ProjectListModel::ProjectHashRole]).toString();

    if (ProjectBackend::instance()->dir(hash).isEmpty())
        goto finish;

    chash = ProjectBackend::instance()->hash();
    if (!chash.isEmpty() && chash == hash) {
        ProjectBackend::instance()->stop();
        chash = "";
    }

    rm(ProjectBackend::instance()->dir(hash));

finish:
    refreshProjectList();
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
            if (!ProjectBackend::instance()->importProject(fileName)) {
                QMessageBox::warning(
                    this,
                    "Operation Stopped",
                    "One or more import file is corrupted."
                );
                return;
            }
        }
        refreshProjectList();
        swipeView->setProperty("currentIndex", 0);
        QMessageBox::information(this, "Finished", "Tool import has successfully finished.");
    }
}

void ProjectsWidget::handleBtnExportClicked()
{
    auto hash = model.get(
        listView->property("currentIndex").toInt(),
        model.roleNames()[ProjectListModel::ProjectHashRole]
    ).toString();

    auto pname = model.get(
        listView->property("currentIndex").toInt(),
        model.roleNames()[ProjectListModel::ProjectNameRole]
    ).toString();

    if (hash.isEmpty() || pname.isEmpty())
        return;

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec()) {
        if (!rm(
            dialog.selectedFiles().at(0) +
            separator() +
            pname + ".zip"
        )) return;

        if (!ProjectBackend::instance()->exportProject(
            hash,
            dialog.selectedFiles().at(0) +
            separator() +
            pname + ".zip"
        )) return;

        QMessageBox::information(
            this,
            "Finished",
            "Project export has successfully finished."
        );
    }
}

void ProjectsWidget::startProject()
{
    auto hash = model.get(
        listView->property("currentIndex").toInt(),
        model.roleNames()[ProjectListModel::ProjectHashRole]
    ).toString();

    if (!ProjectBackend::instance()->start(hash)) { // Asynchronous Operation
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
	auto sizetext = sizeText->property("text").toString();
	auto crdatetext = crDateText->property("text").toString();
	auto ownertext = ownerText->property("text").toString();
    auto prevhash = model.get(listView->property("currentIndex").toInt(),
    model.roleNames()[ProjectListModel::ProjectHashRole]).toString();

    if (projectnametext.isEmpty() || descriptiontext.isEmpty()) {
		QMetaObject::invokeMethod(warning, "show");
		return;
	}

    if (prevhash.isEmpty()) {
        if (!ProjectBackend::instance()->newProject(
            projectnametext,
            descriptiontext,
            ownertext,
            crdatetext,
            sizetext
        )) qFatal("ProjectsWidget::handleBtnOkClicked() : Fatal Error. 0x01");

        ProjectBackend::instance()->updateSize();
    } else {
        ProjectBackend::
        instance()->changeName(
            prevhash,
            projectnametext
        );
        ProjectBackend::
        instance()->changeDescription(
            prevhash,
            descriptiontext
        );
    }

    refreshProjectList();
	swipeView->setProperty("currentIndex", 0);
}

void ProjectsWidget::handleLoadButtonClicked()
{
    auto hash = model.get(listView->property("currentIndex").toInt(),
      model.roleNames()[ProjectListModel::ProjectHashRole]).toString();
    auto chash = ProjectBackend::instance()->hash();

    if (!chash.isEmpty() && chash == hash) {
        emit done();
		return;
    }

    ProjectBackend::instance()->stop();
    QTimer::singleShot(0, this, &ProjectsWidget::startProject);
    emit busy(tr("Loading project"));
}

void ProjectsWidget::refreshProjectList()
{
	model.clear();
    if (UserBackend::instance()->dir().isEmpty())
        return;

    auto projects = ProjectBackend::instance()->projects();

    if (projects.size() < 1)
        return;

    for (auto hash : projects) {
		int lastIndex = model.rowCount();
		model.insertRow(lastIndex);
        model.set(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole],
            ProjectBackend::instance()->name(hash));
        model.set(lastIndex, model.roleNames()[ProjectListModel::ProjectHashRole], hash);
        model.set(lastIndex, model.roleNames()[ProjectListModel::ActiveRole],
            hash == ProjectBackend::instance()->hash());
		model.set(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole],
            "Last edited: " + ProjectBackend::instance()->mfDate(hash));
	}

	listView->setProperty("currentIndex", 0);
}
