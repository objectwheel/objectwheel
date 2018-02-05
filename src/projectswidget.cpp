#include <projectswidget.h>
#include <buttonslice.h>
#include <fit.h>
#include <css.h>
#include <userbackend.h>
#include <projectbackend.h>

#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QScreen>

#define SIZE_LIST        (QSize(fit::fx(400), fit::fx(350)))
#define BUTTONS_WIDTH    (fit::fx(400))
#define SIZE_LOGO        (QSize(fit::fx(80), fit::fx(80)))
#define SIZE_FILEICON    (QSize(fit::fx(48), fit::fx(48)))
#define PATH_LOGO        (":/resources/images/toolbox.png")
#define PATH_FILEICON    (":/resources/images/fileicon.png")
#define PATH_NICON       (":/resources/images/new.png")
#define PATH_LICON       (":/resources/images/load.png")
#define PATH_IICON       (":/resources/images/unload.png")
#define pS               (QApplication::primaryScreen())

enum Buttons { Load, New, Import };
enum { Name = Qt::UserRole + 1, LastEdit, Hash, Active };

class ProjectsDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        ProjectsDelegate(QListWidget* listWidget, QWidget* parent);

        void paint(QPainter* painter, const QStyleOptionViewItem& option,
          const QModelIndex& index) const override;

    private:
        QListWidget* _listWidget;
};

ProjectsDelegate::ProjectsDelegate(QListWidget* listWidget, QWidget* parent) : QStyledItemDelegate(parent)
  , _listWidget(listWidget)
{
}

void ProjectsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   auto item = _listWidget->item(index.row());
   Q_ASSERT(item);

   auto name = item->data(Name).toString();
   auto lastEdit = item->data(LastEdit).toString();

   auto rn = option.rect.adjusted(option.rect.height(),
     fit::fx(7), 0, - option.rect.height() / 2.0);
   auto rl = option.rect.adjusted(option.rect.height(),
     option.rect.height() / 2.0, 0, - fit::fx(7));
   auto ri = option.rect.adjusted(fit::fx(7), fit::fx(7),
     - option.rect.width() + option.rect.height() - fit::fx(7), - fit::fx(7));
   auto icon = item->icon().pixmap(ri.size() * pS->devicePixelRatio());

   painter->setRenderHint(QPainter::Antialiasing);

   QPainterPath path;
   path.addRoundedRect(_listWidget->rect(), fit::fx(8), fit::fx(8));
   painter->setClipPath(path);

    if (item->isSelected())
        painter->fillRect(option.rect, option.palette.highlight());

    painter->drawPixmap(ri, icon, icon.rect());

    QFont f;
    f.setWeight(QFont::DemiBold);
    painter->setFont(f);
    painter->setPen("#21303c");
    painter->drawText(rn, name, Qt::AlignVCenter | Qt::AlignLeft);

    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->drawText(rl, lastEdit, Qt::AlignVCenter | Qt::AlignLeft);
}

ProjectsWidget::ProjectsWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _iconLabel = new QLabel;
    _welcomeLabel = new QLabel;
    _versionLabel = new QLabel;
    _projectsLabel = new QLabel;
    _listWidget = new QListWidget;
    _buttons = new ButtonSlice;

    _layout->addStretch();
    _layout->setSpacing(fit::fx(12));
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_iconLabel, 0, Qt::AlignCenter);
    _layout->addWidget(_welcomeLabel, 0, Qt::AlignCenter);
    _layout->addWidget(_versionLabel, 0, Qt::AlignCenter);
    _layout->addSpacing(fit::fx(10));
    _layout->addWidget(_projectsLabel, 0, Qt::AlignCenter);
    _layout->addWidget(_listWidget, 0, Qt::AlignCenter);
    _layout->addWidget(_buttons, 0, Qt::AlignCenter);
    _layout->addStretch();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(pS->devicePixelRatio());

    _iconLabel->setFixedSize(SIZE_LOGO);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_LOGO * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(fit::fx(28));

    _welcomeLabel->setFont(f);
    _welcomeLabel->setText(tr("Welcome to Objectwheel"));
    _welcomeLabel->setStyleSheet("color: #2E3A41");

    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(15));
    _versionLabel->setFont(f);
    _versionLabel->setText(tr("Version ") + tr(APP_VER) + " (" APP_GITHASH ")");
    _versionLabel->setStyleSheet("color: #2E3A41");

    _projectsLabel->setText(tr("Your Projects"));
    _projectsLabel->setStyleSheet("color: #2E3A41");

    QPalette p1;
    p1.setColor(QPalette::Highlight, "#12000000");
    _listWidget->viewport()->installEventFilter(this);
    _listWidget->setPalette(p1);
    _listWidget->setIconSize(SIZE_FILEICON);
    _listWidget->setMinimumWidth(fit::fx(400));
    _listWidget->setItemDelegate(new ProjectsDelegate(_listWidget, _listWidget));
    _listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget->setFocusPolicy(Qt::NoFocus);
    _listWidget->setFixedSize(SIZE_LIST);
    _listWidget->verticalScrollBar()->setStyleSheet(
        tr(
            "QScrollBar:vertical {"
            "    background: transparent;"
            "    width: %2px;"
            "} QScrollBar::handle:vertical {"
            "    background: #909497;"
            "    min-height: %1px;"
            "    border-radius: %3px;"
            "} QScrollBar::add-line:vertical {"
            "    background: none;"
            "} QScrollBar::sub-line:vertical {"
            "    background: none;"
            "} QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
            "    background: none;"
            "} QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
            "    background: none;"
            "}"
        ).
        arg(fit::fx(15)).
        arg(fit::fx(6)).
        arg(fit::fx(2.5))
    );
    _listWidget->setStyleSheet(
        tr(
            "QListWidget {"
            "    background: #12000000;"
            "    border: 1px solid #22000000;"
            "    border-radius: %1px;"
            "}"
        )
        .arg(fit::fx(8))
    );

    for (int i = 0; i < 10; i++) {
        auto item = new QListWidgetItem("Test");
        item->setIcon(QIcon(PATH_FILEICON));
        item->setData(Name, "Project - 1");
        item->setData(LastEdit, "Last edit: 2018.06.16 14:43:11");
        _listWidget->addItem(item);
    }

    _buttons->add(New, "#CC5D67", "#B2525A");
    _buttons->add(Load, "#5BC5F8", "#2592F9");
    _buttons->add(Import, "#8BBB56", "#6EA045");

    _buttons->get(New)->setText(tr("New"));
    _buttons->get(Load)->setText(tr("Load"));
    _buttons->get(Import)->setText(tr("Import"));

    _buttons->get(New)->setIcon(QIcon(PATH_NICON));
    _buttons->get(Load)->setIcon(QIcon(PATH_LICON));
    _buttons->get(Import)->setIcon(QIcon(PATH_IICON));

    _buttons->get(New)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Load)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Import)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 3.0;
    _buttons->triggerSettings();

}

bool ProjectsWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _listWidget->viewport() && event->type() == QEvent::Paint && _listWidget->count() == 0) {
        QPainter p(_listWidget->viewport());
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen("#30000000");
        p.drawText(_listWidget->viewport()->rect(), tr("No projects"), QTextOption(Qt::AlignCenter));
        return true;
    }

    return false;
}

//void ProjectsWidget::handleNewButtonClicked()
//{
//    if (UserBackend::instance()->dir().isEmpty()) return;
//    auto projects = ProjectBackend::instance()->projectNames();
//    int count = 1;
//    QString projectName = "Project - 1";
//    while (projects.contains(projectName)) {
//        count++;
//        projectName.remove(projectName.size() - 1, 1);
//        projectName += QString::number(count);
//    }

//    int lastIndex = model.rowCount();
//    model.insertRow(lastIndex);
//    listView->setProperty("currentIndex", lastIndex);
//    model.set(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole], projectName);
//    model.set(lastIndex, model.roleNames()[ProjectListModel::ActiveRole], false);
//    model.set(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole],
//            QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " "));

//    sizeText->setProperty("text", "0 bytes");
//    mfDateText->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole]));
//    crDateText->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::LastEditedRole]));
//    ownerText->setProperty("text", UserBackend::instance()->user());
//    descriptionTextInput->setProperty("text", "Simple description here.");
//    projectnameTextInput->setProperty("text", model.get(lastIndex, model.roleNames()[ProjectListModel::ProjectNameRole]));
//    QTimer::singleShot(250, [=]{ swipeView->setProperty("currentIndex", 1); });
//}

//void ProjectsWidget::handleInfoButtonClicks(const QVariant& hash)
//{
//    auto h = hash.toString();
//    ProjectBackend::instance()->updateSize();
//    sizeText->setProperty("text", ProjectBackend::instance()->size(h));
//    mfDateText->setProperty("text", ProjectBackend::instance()->mfDate(h));
//    crDateText->setProperty("text", ProjectBackend::instance()->crDate(h));
//    ownerText->setProperty("text", ProjectBackend::instance()->owner(h));
//    descriptionTextInput->setProperty("text", ProjectBackend::instance()->description(h));
//    projectnameTextInput->setProperty("text", ProjectBackend::instance()->name(h));
//    swipeView->setProperty("currentIndex", 1);
//}

//void ProjectsWidget::handleBtnCancelClicked()
//{
//    refreshProjectList();
//    swipeView->setProperty("currentIndex", 0);
//}

//void ProjectsWidget::handleBtnDeleteClicked()
//{
//    QString chash;
//    auto hash = model.get(listView->property("currentIndex").toInt(),
//      model.roleNames()[ProjectListModel::ProjectHashRole]).toString();

//    if (ProjectBackend::instance()->dir(hash).isEmpty())
//        goto finish;

//    chash = ProjectBackend::instance()->hash();
//    if (!chash.isEmpty() && chash == hash) {
//        ProjectBackend::instance()->stop();
//        chash = "";
//    }

//    rm(ProjectBackend::instance()->dir(hash));

//finish:
//    refreshProjectList();
//    swipeView->setProperty("currentIndex", 0);
//}

//void ProjectsWidget::handleBtnImportClicked()
//{
//    QFileDialog dialog(this);
//    dialog.setFileMode(QFileDialog::ExistingFiles);
//    dialog.setNameFilter(tr("Zip files (*.zip)"));
//    dialog.setViewMode(QFileDialog::Detail);

//    if (dialog.exec()) {
//        for (auto fileName : dialog.selectedFiles()) {
//            if (!ProjectBackend::instance()->importProject(fileName)) {
//                QMessageBox::warning(
//                    this,
//                    "Operation Stopped",
//                    "One or more import file is corrupted."
//                );
//                return;
//            }
//        }
//        refreshProjectList();
//        swipeView->setProperty("currentIndex", 0);
//        QMessageBox::information(this, "Finished", "Tool import has successfully finished.");
//    }
//}

//void ProjectsWidget::handleBtnExportClicked()
//{
//    auto hash = model.get(
//        listView->property("currentIndex").toInt(),
//        model.roleNames()[ProjectListModel::ProjectHashRole]
//    ).toString();

//    auto pname = model.get(
//        listView->property("currentIndex").toInt(),
//        model.roleNames()[ProjectListModel::ProjectNameRole]
//    ).toString();

//    if (hash.isEmpty() || pname.isEmpty())
//        return;

//    QFileDialog dialog(this);
//    dialog.setFileMode(QFileDialog::Directory);
//    dialog.setViewMode(QFileDialog::Detail);
//    dialog.setOption(QFileDialog::ShowDirsOnly, true);

//    if (dialog.exec()) {
//        if (!rm(
//            dialog.selectedFiles().at(0) +
//            separator() +
//            pname + ".zip"
//        )) return;

//        if (!ProjectBackend::instance()->exportProject(
//            hash,
//            dialog.selectedFiles().at(0) +
//            separator() +
//            pname + ".zip"
//        )) return;

//        QMessageBox::information(
//            this,
//            "Finished",
//            "Project export has successfully finished."
//        );
//    }
//}

//void ProjectsWidget::startProject()
//{
//    auto hash = model.get(
//        listView->property("currentIndex").toInt(),
//        model.roleNames()[ProjectListModel::ProjectHashRole]
//    ).toString();

//    if (!ProjectBackend::instance()->start(hash)) { // Asynchronous Operation
//        for (int i = model.rowCount(); i--;) {
//            if (model.get(i, model.roleNames()[ProjectListModel::ActiveRole]).toBool()) {
//                model.set(i, model.roleNames()[ProjectListModel::ActiveRole], false);
//            }
//        }
//        qFatal("Fatal : ProjectsWidget");
//    }

//    for (int i = model.rowCount(); i--;) {
//        if (model.get(i, model.roleNames()[ProjectListModel::ActiveRole]).toBool()) {
//            model.set(i, model.roleNames()[ProjectListModel::ActiveRole], false);
//        }
//    }

//    model.set(listView->property("currentIndex").toInt(),
//        model.roleNames()[ProjectListModel::ActiveRole], true);

//    emit done();
//}

//void ProjectsWidget::handleBtnOkClicked()
//{
//    auto projectnametext = projectnameTextInput->property("text").toString();
//    auto descriptiontext = descriptionTextInput->property("text").toString();
//    auto sizetext = sizeText->property("text").toString();
//    auto crdatetext = crDateText->property("text").toString();
//    auto ownertext = ownerText->property("text").toString();
//    auto prevhash = model.get(listView->property("currentIndex").toInt(),
//    model.roleNames()[ProjectListModel::ProjectHashRole]).toString();

//    if (projectnametext.isEmpty() || descriptiontext.isEmpty()) {
//        QMetaObject::invokeMethod(warning, "show");
//        return;
//    }

//    if (prevhash.isEmpty()) {
//        if (!ProjectBackend::instance()->newProject(
//            projectnametext,
//            descriptiontext,
//            ownertext,
//            crdatetext,
//            sizetext
//        )) qFatal("ProjectsWidget::handleBtnOkClicked() : Fatal Error. 0x01");

//        ProjectBackend::instance()->updateSize();
//    } else {
//        ProjectBackend::
//        instance()->changeName(
//            prevhash,
//            projectnametext
//        );
//        ProjectBackend::
//        instance()->changeDescription(
//            prevhash,
//            descriptiontext
//        );
//    }

//    refreshProjectList();
//    swipeView->setProperty("currentIndex", 0);
//}

//void ProjectsWidget::handleLoadButtonClicked()
//{
//    auto hash = model.get(listView->property("currentIndex").toInt(),
//      model.roleNames()[ProjectListModel::ProjectHashRole]).toString();
//    auto chash = ProjectBackend::instance()->hash();

//    if (!chash.isEmpty() && chash == hash) {
//        emit done();
//        return;
//    }

//    if (dW->qmlEditorView()->hasUnsavedDocs()) {
//        QMessageBox msgBox;
//        msgBox.setText(tr("%1 has some unsaved documents.").arg(ProjectBackend::instance()->name()));
//        msgBox.setInformativeText("Do you want to save all your changes, or cancel loading new project?");
//        msgBox.setStandardButtons(QMessageBox::SaveAll | QMessageBox::NoToAll | QMessageBox::Cancel);
//        msgBox.setDefaultButton(QMessageBox::SaveAll);
//        int ret = msgBox.exec();

//        switch (ret) {
//            case QMessageBox::Cancel:
//                return;

//            case QMessageBox::SaveAll:
//                dW->qmlEditorView()->saveAll();
//                break;

//            case QMessageBox::NoToAll:
//                break;
//        }
//    }

//    WindowManager::instance()->hide(WindowManager::Main);
//    ProjectBackend::instance()->stop();
//    QTimer::singleShot(0, this, &ProjectsWidget::startProject);
//    emit busy(tr("Loading project"));
//}

void ProjectsWidget::refreshProjectList()
{
    _listWidget->clear();
    if (UserBackend::instance()->dir().isEmpty())
        return;

    auto projects = ProjectBackend::instance()->projects();

    if (projects.size() < 1)
        return;

    for (auto hash : projects) {
        auto item = new QListWidgetItem;
        item->setIcon(QIcon(PATH_FILEICON));
        item->setData(Hash, hash);
        item->setData(Name, ProjectBackend::instance()->name(hash));
        item->setData(LastEdit, tr("Last edit: ") + ProjectBackend::instance()->mfDate(hash));
        item->setData(Active, hash == ProjectBackend::instance()->hash());
        _listWidget->addItem(item);
    }

//    listView->setProperty("currentIndex", 0);
}

#include "projectswidget.moc"
