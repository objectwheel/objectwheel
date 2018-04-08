#include <projectswidget.h>
#include <buttonslice.h>
#include <fit.h>
#include <css.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <qmlcodeeditorwidget.h>
#include <delayer.h>
#include <filemanager.h>
#include <previewerbackend.h>
#include <savebackend.h>
#include <progressbar.h>
#include <windowmanager.h>

#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QListWidget>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QDebug>
#include <QDateTime>
#include <QGuiApplication>
#include <QScreen>
#include <QFileDialog>

#define SIZE_LIST        (QSize(fit::fx(450), fit::fx(350)))
#define BUTTONS_WIDTH    (fit::fx(450))
#define SIZE_LOGO        (QSize(fit::fx(80), fit::fx(80)))
#define SIZE_FILEICON    (QSize(fit::fx(48), fit::fx(48)))
#define PATH_LOGO        (":/resources/images/toolbox.png")
#define PATH_FILEICON    (":/resources/images/fileicon.png")
#define PATH_NICON       (":/resources/images/new.png")
#define PATH_LICON       (":/resources/images/ok.png")
#define PATH_IICON       (":/resources/images/load.png")
#define PATH_EICON       (":/resources/images/unload.png")
#define PATH_SICON       (":/resources/images/dots.png")
#define WIDTH_PROGRESS   80
#define DPR              QGuiApplication::primaryScreen()->devicePixelRatio()
#define TIME             QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate)

enum Buttons { Load, New, Import, Export, Settings };
enum Roles { Name = Qt::UserRole + 1, LastEdit, Hash, Active };

namespace { int totalTask = 0; }

class ProjectsDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        ProjectsDelegate(QListWidget* listWidget, QWidget* parent);

        void paint(QPainter* painter, const QStyleOptionViewItem& option,
          const QModelIndex& index) const override;

    private:
        QListWidget* m_listWidget;
};

ProjectsDelegate::ProjectsDelegate(QListWidget* listWidget, QWidget* parent) : QStyledItemDelegate(parent)
  , m_listWidget(listWidget)
{
}

void ProjectsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   auto item = m_listWidget->item(index.row());
   Q_ASSERT(item);

   auto name = item->data(Name).toString();
   auto lastEdit = item->data(LastEdit).toString();

   auto rn = option.rect.adjusted(option.rect.height(),
     fit::fx(7), 0, - option.rect.height() / 2.0);
   auto rl = option.rect.adjusted(option.rect.height(),
     option.rect.height() / 2.0, 0, - fit::fx(7));
   auto ri = option.rect.adjusted(fit::fx(7), fit::fx(7),
     - option.rect.width() + option.rect.height() - fit::fx(7), - fit::fx(7));
   auto ra = ri.adjusted(fit::fx(3), fit::fx(-0.5), 0, 0);
   ra.setSize(fit::fx(QSizeF(10, 10)).toSize());
   auto icon = item->icon().pixmap(ri.size() * DPR);

   painter->setRenderHint(QPainter::Antialiasing);

   QPainterPath path;
   path.addRoundedRect(m_listWidget->rect(), fit::fx(8), fit::fx(8));
   painter->setClipPath(path);

    if (item->isSelected())
        painter->fillRect(option.rect, option.palette.highlight());

    painter->drawPixmap(ri, icon, icon.rect());

    if (item->data(Active).toBool()) {
        QLinearGradient g(ri.topLeft(), ri.bottomLeft());
        g.setColorAt(0, "#6BCB36");
        g.setColorAt(0.5, "#4db025");
        painter->setBrush(g);
        painter->setPen("#6BCB36");
        painter->drawRoundedRect(ra, ra.width(), ra.height());
    }

    QFont f;
    f.setWeight(QFont::DemiBold);
    painter->setFont(f);
    painter->setPen("#21303c");
    painter->drawText(rn, name, Qt::AlignVCenter | Qt::AlignLeft);

    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->drawText(rl, tr("Last Edit: ") + lastEdit, Qt::AlignVCenter | Qt::AlignLeft);
}

ProjectsWidget::ProjectsWidget(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_iconLabel(new QLabel)
  , m_welcomeLabel(new QLabel)
  , m_versionLabel(new QLabel)
  , m_projectsLabel(new QLabel)
  , m_listWidget(new QListWidget)
  , m_buttons(new ButtonSlice)
  , m_buttons_2(new ButtonSlice(m_listWidget->viewport()))
  , m_progressBar(new ProgressBar(m_listWidget->viewport()))
{
    m_layout->addStretch();
    m_layout->setSpacing(fit::fx(12));
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_welcomeLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_versionLabel, 0, Qt::AlignCenter);
    m_layout->addSpacing(fit::fx(10));
    m_layout->addWidget(m_projectsLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_listWidget, 0, Qt::AlignCenter);
    m_layout->addWidget(m_buttons, 0, Qt::AlignCenter);
    m_layout->addStretch();

    m_progressBar->hide();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(DPR);

    m_iconLabel->setFixedSize(SIZE_LOGO);
    m_iconLabel->setPixmap(
        p.scaled(
            SIZE_LOGO * DPR,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(fit::fx(28));

    m_welcomeLabel->setFont(f);
    m_welcomeLabel->setText(tr("Welcome to Objectwheel"));
    m_welcomeLabel->setStyleSheet("color: black");

    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));
    m_versionLabel->setFont(f);
    m_versionLabel->setText(tr("Version ") + tr(APP_VER) + " (" APP_GITHASH ")");
    m_versionLabel->setStyleSheet("color: black");

    m_projectsLabel->setText(tr("Your Projects"));
    m_projectsLabel->setStyleSheet("color: black");

    QPalette p1;
    p1.setColor(QPalette::Highlight, "#12000000");
    m_listWidget->viewport()->installEventFilter(this);
    m_listWidget->setPalette(p1);
    m_listWidget->setIconSize(SIZE_FILEICON);
    m_listWidget->setMinimumWidth(fit::fx(400));
    m_listWidget->setItemDelegate(new ProjectsDelegate(m_listWidget, m_listWidget));
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setFixedSize(SIZE_LIST);
    m_listWidget->verticalScrollBar()->setStyleSheet(
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
    m_listWidget->setStyleSheet(
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
        m_listWidget->addItem(item);
    }

    m_progressBar->setFixedWidth(WIDTH_PROGRESS);

    m_buttons_2->setFixedHeight(fit::fx(20));
    m_buttons_2->add(Settings, "#55A6F6", "#448DDE");
    m_buttons_2->get(Settings)->setIconSize(fit::fx(QSizeF(12, 12)).toSize());
    m_buttons_2->get(Settings)->setIcon(QIcon(PATH_SICON));
    m_buttons_2->get(Settings)->setCursor(Qt::PointingHandCursor);
    m_buttons_2->hide();
    m_buttons_2->settings().cellWidth = m_buttons_2->height();
    m_buttons_2->settings().borderRadius = m_buttons_2->height() / 2.0;
    m_buttons_2->triggerSettings();
    connect(m_listWidget, &QListWidget::currentItemChanged, [=] {
        auto currentItem = m_listWidget->currentItem();
        if (currentItem) {
            m_buttons_2->show();
            auto rect = m_listWidget->visualItemRect(currentItem);
            m_buttons_2->move(rect.topRight().x() - m_buttons_2->width() - fit::fx(5),
                            rect.topRight().y() + (rect.height() - m_buttons_2->height()) / 2.0);
        } else {
            m_buttons_2->hide();
        }
    });

    m_buttons->add(New, "#B97CD3", "#985BB2");
    m_buttons->add(Load, "#5BC5F8", "#2592F9");
    m_buttons->add(Import, "#8BBB56", "#6EA045");
    m_buttons->add(Export, "#AA815A", "#8c6b4a");
    m_buttons->get(New)->setText(tr("New"));
    m_buttons->get(Load)->setText(tr("Load"));
    m_buttons->get(Import)->setText(tr("Import"));
    m_buttons->get(Export)->setText(tr("Export"));
    m_buttons->get(New)->setIcon(QIcon(PATH_NICON));
    m_buttons->get(Load)->setIcon(QIcon(PATH_LICON));
    m_buttons->get(Import)->setIcon(QIcon(PATH_IICON));
    m_buttons->get(Export)->setIcon(QIcon(PATH_EICON));
    m_buttons->get(New)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Load)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Import)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Export)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = BUTTONS_WIDTH / 4.0;
    m_buttons->triggerSettings();

    connect(m_buttons->get(New), SIGNAL(clicked(bool)),
      this, SLOT(onNewButtonClick()));
    connect(m_buttons->get(Load), SIGNAL(clicked(bool)),
      this, SLOT(onLoadButtonClick()));
    connect(m_buttons->get(Import), SIGNAL(clicked(bool)),
      this, SLOT(onImportButtonClick()));
    connect(m_buttons->get(Export), SIGNAL(clicked(bool)),
      this, SLOT(onExportButtonClick()));
    connect(m_buttons_2->get(Settings), SIGNAL(clicked(bool)),
      this, SLOT(onSettingsButtonClick()));

    connect(PreviewerBackend::instance(), SIGNAL(taskDone()),
      SLOT(onProgressChange()));
    connect(ProjectBackend::instance(), &ProjectBackend::started, [=]
    {
        totalTask = PreviewerBackend::instance()->totalTask();
        onProgressChange();
    });
}

bool ProjectsWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_listWidget->viewport() && event->type() == QEvent::Paint && m_listWidget->count() == 0) {
        QPainter p(m_listWidget->viewport());
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen("#30000000");
        p.drawText(m_listWidget->viewport()->rect(), tr("No projects"), QTextOption(Qt::AlignCenter));
        return true;
    }

    return false;
}

void ProjectsWidget::refreshProjectList()
{
    const int currentRow = m_listWidget->currentRow();
    m_listWidget->clear();
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
        item->setData(LastEdit, ProjectBackend::instance()->mfDate(hash));
        item->setData(Active, hash == ProjectBackend::instance()->hash());
        m_listWidget->addItem(item);
    }

    if (currentRow >= 0 && m_listWidget->count() > currentRow)
        m_listWidget->setCurrentRow(currentRow);
    else
        m_listWidget->setCurrentRow(0);
}

void ProjectsWidget::startProject()
{
    auto hash = m_listWidget->currentItem()->data(Hash).toString();

    if (!ProjectBackend::instance()->start(hash)) {
        qWarning() << "Project starting unsuccessful.";
        refreshProjectList();
        return;
    }

    for (int i = m_listWidget->count(); i--;)
        m_listWidget->item(i)->setData(Active, false);

    m_listWidget->currentItem()->setData(Active, true);

    Delayer::delay(std::bind(&PreviewerBackend::isBusy, PreviewerBackend::instance()));
    unlock();
    emit done();
}

void ProjectsWidget::onNewButtonClick()
{
    if (UserBackend::instance()->dir().isEmpty()) return;
    auto projects = ProjectBackend::instance()->projectNames();
    int count = 1;
    QString projectName = "Project - 1";

    m_buttons->setDisabled(true);

    while (projects.contains(projectName)) {
        count++;
        projectName.remove(projectName.size() - 1, 1);
        projectName += QString::number(count);
    }

    auto item = new QListWidgetItem;
    item->setIcon(QIcon(PATH_FILEICON));
    item->setData(Name, projectName);
    item->setData(LastEdit, TIME);
    item->setData(Active, false);
    m_listWidget->addItem(item);
    m_listWidget->setCurrentItem(item);

    Delayer::delay(250);

    m_buttons->setEnabled(true);

    emit newProject(projectName);
}

void ProjectsWidget::onLoadButtonClick()
{
    if (!m_listWidget->currentItem()) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Select a project first.")
        );
        return;
    }

    auto hash = m_listWidget->currentItem()->data(Hash).toString();
    auto chash = ProjectBackend::instance()->hash();

    if (!chash.isEmpty() && chash == hash) {
        emit done();
        return;
    }

// FIXME
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

    WindowManager::instance()->hide(WindowManager::Main);
    ProjectBackend::instance()->stop();
    QTimer::singleShot(0, this, &ProjectsWidget::startProject);

    lock();
}

void ProjectsWidget::onExportButtonClick()
{
    if (!m_listWidget->currentItem()) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Select the project first.")
        );
        return;
    }

    auto hash = m_listWidget->currentItem()->data(Hash).toString();
    auto pname = m_listWidget->currentItem()->data(Name).toString();

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

void ProjectsWidget::onImportButtonClick()
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
        QMessageBox::information(
            this,
            tr("Finished"),
            tr("Tool import has successfully finished.")
        );
    }
}

void ProjectsWidget::onSettingsButtonClick()
{
    if (m_listWidget->currentItem())
        emit editProject(m_listWidget->currentItem()->data(Hash).toString());
}

void ProjectsWidget::onProgressChange()
{
    int taskDone = totalTask - PreviewerBackend::instance()->totalTask();
    m_progressBar->setValue(10 + 90.0 * taskDone / totalTask);
}

void ProjectsWidget::lock()
{
    m_buttons->setDisabled(true);
    m_listWidget->setDisabled(true);
    m_buttons_2->hide();

    m_progressBar->move(
        m_buttons_2->pos() +
        QPoint(
            -WIDTH_PROGRESS + m_buttons_2->width(),
            m_buttons_2->height() / 2.0 - m_progressBar->height() / 2.0
        )
    );

    m_progressBar->show();
    m_progressBar->raise();
    m_progressBar->setValue(10);
}

void ProjectsWidget::unlock()
{
    m_buttons->setEnabled(true);
    m_listWidget->setEnabled(true);
    m_buttons_2->show();
    m_progressBar->hide();
}

#include "projectswidget.moc"
