#include <projectswidget.h>
#include <buttonslice.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <delayer.h>
#include <filemanager.h>
#include <controlpreviewingmanager.h>
#include <progressbar.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <utilityfunctions.h>
#include <transparentstyle.h>

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
#include <QScreen>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSortFilterProxyModel>
#include <QDateTime>

#define SIZE_LIST        (QSize(450, 300))
#define BUTTONS_WIDTH    (450)
#define SIZE_LOGO        (QSize(48, 48))
#define SIZE_FILEICON    (QSize(48, 48))
#define PATH_LOGO        (":/images/toolbox.png")
#define PATH_FILEICON    (":/images/fileicon.png")
#define PATH_NICON       (":/images/new.png")
#define PATH_LICON       (":/images/ok.png")
#define PATH_IICON       (":/images/load.png")
#define PATH_EICON       (":/images/unload.png")
#define PATH_SICON       (":/images/dots.png")
#define WIDTH_PROGRESS   80

enum Buttons { Load, New, Import, Export, Settings };
enum Roles { Name = Qt::UserRole + 1, LastEdit, Hash, Active };

class FilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterWidget(QWidget* parent = nullptr) : QWidget(parent)
      , m_layout(new QHBoxLayout(this))
      , m_sortLabel(new QLabel)
      , m_sortComboBox(new QComboBox)
      , m_filterLineEdit(new QLineEdit)
      , m_reverseSortCheckBox(new QCheckBox)
    {
        m_layout->setContentsMargins(2, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(m_filterLineEdit);
        m_layout->addWidget(m_sortLabel);
        m_layout->addWidget(m_reverseSortCheckBox);
        m_layout->addWidget(m_sortComboBox);

        TransparentStyle::attach(this);
        QTimer::singleShot(100, [=] { // FIXME
            TransparentStyle::attach(this);
        });

        m_sortLabel->setText("| " + tr("Sort reverse "));

        m_filterLineEdit->setPlaceholderText(tr("Filter"));
        m_filterLineEdit->setStyleSheet("border: none; background: transparent;");
        m_filterLineEdit->setClearButtonEnabled(true);
        m_filterLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        m_filterLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_filterLineEdit->setToolTip(tr("Filter by name"));

        m_sortComboBox->addItem(tr("Date"));
        m_sortComboBox->addItem(tr("Name"));
        m_sortComboBox->setCursor(Qt::PointingHandCursor);
        m_sortComboBox->setToolTip(tr("Change sorting criteria"));
        m_sortComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        m_sortComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        m_reverseSortCheckBox->setText("by :");
        m_reverseSortCheckBox->setToolTip(tr("Reverse order"));
        m_reverseSortCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_reverseSortCheckBox->setCursor(Qt::PointingHandCursor);

        connect(m_filterLineEdit, &QLineEdit::textChanged, this, &FilterWidget::filterTextChanged);
        connect(m_sortComboBox, &QComboBox::currentTextChanged, this, &FilterWidget::sortCriteriaChanged);
        connect(m_reverseSortCheckBox, &QCheckBox::clicked, this, [=] {
            emit sortCriteriaChanged(m_sortComboBox->currentText());
        });
    }

    bool isReverseSort() const
    {
        return m_reverseSortCheckBox->isChecked();
    }

    QString sortingCriteria() const
    {
        return m_sortComboBox->currentText();
    }

public slots:
    void textFocus()
    {
        m_filterLineEdit->setFocus();
    }

private:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setPen("#22000000");
        p.setBrush(QColor("#12000000"));
        p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 1.0, 1.0);
    }

    QSize sizeHint() const override
    {
        return QSize(100, 24);
    }

signals:
    void filterTextChanged(const QString& text);
    void sortCriteriaChanged(const QString& criteria);

private:
    QHBoxLayout* m_layout;
    QLabel* m_sortLabel;
    QComboBox* m_sortComboBox;
    QLineEdit* m_filterLineEdit;
    QCheckBox* m_reverseSortCheckBox;
};

class ProjectsDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    ProjectsDelegate(QListWidget* listWidget, QWidget* parent) : QStyledItemDelegate(parent)
      , m_listWidget(listWidget)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        auto item = m_listWidget->item(index.row());
        Q_ASSERT(item);

        auto name = item->data(Name).toString();
        auto lastEdit = item->data(LastEdit).toString();

        auto rn = QRectF(option.rect).adjusted(option.rect.height(),
                                               7, 0, - option.rect.height() / 2.0);
        auto rl = QRectF(option.rect).adjusted(option.rect.height(),
                                               option.rect.height() / 2.0, 0, - 7);
        auto ri = QRectF(option.rect).adjusted(7, 7,
                                               - option.rect.width() + option.rect.height() - 7, - 7);
        auto ra = ri.adjusted(3, -0.5, 0, 0);
        ra.setSize(QSize(10, 10));
        auto icon = item->icon().pixmap(UtilityFunctions::window(m_listWidget), ri.size().toSize(),
                                        m_listWidget->isEnabled() ? QIcon::Normal : QIcon::Disabled);

        painter->setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(m_listWidget->rect(), 8, 8);
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
        painter->setPen(option.palette.text().color());
        painter->drawText(rn, name, Qt::AlignVCenter | Qt::AlignLeft);

        f.setWeight(QFont::Normal);
        painter->setFont(f);
        painter->drawText(rl, tr("Last Edit: ") + lastEdit, Qt::AlignVCenter | Qt::AlignLeft);
    }


private:
    QListWidget* m_listWidget;
};

class ProjectListWidgetItem : public QListWidgetItem
{
public:
    explicit ProjectListWidgetItem(ProjectsWidget* view) : m_projectsWidget(view)
    {}

    bool operator<(const QListWidgetItem& other) const override
    {
        const FilterWidget* filterWidget = m_projectsWidget->filterWidget();
        const QString& criteria = filterWidget->sortingCriteria();
        bool reverseSort = filterWidget->isReverseSort();

        if (criteria == QObject::tr("Name")) {
            const QString& myName = data(Name).toString();
            const QString& othersName = other.data(Name).toString();
            bool result = QString::localeAwareCompare(myName, othersName) < 0;
            return reverseSort ? !result : result;
        }
        if (criteria == QObject::tr("Date")) {
            const QDateTime& myDate = ProjectManager::fromUi(data(LastEdit).toString());
            const QDateTime& othersDate = ProjectManager::fromUi(other.data(LastEdit).toString());
            bool result = myDate > othersDate;
            return reverseSort ? !result : result;
        }

        return QListWidgetItem::operator <(other);
    }

private:
    ProjectsWidget* m_projectsWidget;
};

ProjectsWidget::ProjectsWidget(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_iconLabel(new QLabel)
  , m_welcomeLabel(new QLabel)
  , m_versionLabel(new QLabel)
  , m_projectsLabel(new QLabel)
  , m_filterWidget(new FilterWidget)
  , m_listWidget(new QListWidget)
  , m_buttons(new ButtonSlice)
  , m_buttons_2(new ButtonSlice(m_listWidget->viewport()))
  , m_progressBar(new ProgressBar(m_listWidget->viewport()))
{
    m_layout->addStretch();
    m_layout->setSpacing(6);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_welcomeLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_versionLabel, 0, Qt::AlignCenter);
    m_layout->addSpacing(6);
    m_layout->addWidget(m_projectsLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_filterWidget, 0, Qt::AlignCenter);
    m_layout->addWidget(m_listWidget, 0, Qt::AlignCenter);
    m_layout->addWidget(m_buttons, 0, Qt::AlignCenter);
    m_layout->addStretch();

    m_progressBar->hide();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(devicePixelRatioF());

    m_iconLabel->setFixedSize(SIZE_LOGO);
    m_iconLabel->setPixmap(
                p.scaled(
                    SIZE_LOGO * devicePixelRatioF(),
                    Qt::IgnoreAspectRatio,
                    Qt::SmoothTransformation
                    )
                );

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(26);

    m_welcomeLabel->setFont(f);
    m_welcomeLabel->setText(tr("Welcome to Objectwheel"));

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_versionLabel->setFont(f);
    m_versionLabel->setText(tr("Version ") + tr(APP_VER) + " (" APP_GITHASH ")");

    m_projectsLabel->setText(tr("Your Projects"));

    m_filterWidget->setFixedWidth(SIZE_LIST.width());
    connect(m_filterWidget, &FilterWidget::filterTextChanged, this, &ProjectsWidget::onFilterTextChange);
    connect(m_filterWidget, &FilterWidget::sortCriteriaChanged, [=] {
        m_listWidget->sortItems();
    });

    QPalette p1;
    p1.setColor(QPalette::Highlight, "#15000000");
    m_listWidget->setPalette(p1);
    m_listWidget->viewport()->installEventFilter(this);
    m_listWidget->setIconSize(SIZE_FILEICON);
    m_listWidget->setMinimumWidth(400);
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
                arg(15).
                arg(6).
                arg(2.5)
                );
    m_listWidget->setStyleSheet(
                tr(
                    "QListWidget {"
                    "    background: #12000000;"
                    "    border: 1px solid #22000000;"
                    "    border-radius: %1px;"
                    "}"
                    )
                .arg(8)
                );

    m_progressBar->setFixedWidth(WIDTH_PROGRESS);

    m_buttons_2->setFixedHeight(20);
    m_buttons_2->add(Settings, "#55A6F6", "#448DDE");
    m_buttons_2->get(Settings)->setIconSize(QSize(12, 12));
    m_buttons_2->get(Settings)->setIcon(QIcon(PATH_SICON));
    m_buttons_2->get(Settings)->setCursor(Qt::PointingHandCursor);
    m_buttons_2->hide();
    m_buttons_2->settings().cellWidth = m_buttons_2->height();
    m_buttons_2->settings().borderRadius = m_buttons_2->height() / 2.0;
    m_buttons_2->triggerSettings();
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &ProjectsWidget::onLoadButtonClick);
    connect(m_listWidget, &QListWidget::currentItemChanged, [=] {
        auto currentItem = m_listWidget->currentItem();
        if (currentItem) {
            m_buttons_2->show();
            auto rect = m_listWidget->visualItemRect(currentItem);
            m_buttons_2->move(rect.topRight().x() - m_buttons_2->width() - 5,
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
    m_buttons->get(New)->setIconSize(QSize(16, 16));
    m_buttons->get(Load)->setIconSize(QSize(16, 16));
    m_buttons->get(Import)->setIconSize(QSize(16, 16));
    m_buttons->get(Export)->setIconSize(QSize(16, 16));
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

    connect(ControlPreviewingManager::instance(), &ControlPreviewingManager::initializationProgressChanged,
            this, &ProjectsWidget::onProgressChange);
}

bool ProjectsWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_listWidget->viewport()) {
        if(event->type() == QEvent::Paint) {
            QString message = tr("No results");
            if (m_listWidget->count() == 0)
                message = tr("No projects");

            QListWidgetItem* firstVisibleItem = nullptr;
            for (int i = m_listWidget->count() - 1; i >= 0; --i) {
                QListWidgetItem* item = m_listWidget->item(i);
                if (!item->isHidden()) {
                    firstVisibleItem = item;
                    break;
                }
            }
            if (!firstVisibleItem) {
                QPainter p(m_listWidget->viewport());
                p.setRenderHint(QPainter::Antialiasing);
                p.setPen("#45000000");
                p.drawText(m_listWidget->viewport()->rect(), message, QTextOption(Qt::AlignCenter));
                return true;
            }
        }
        if(event->type() == QEvent::MouseButtonPress)
            m_filterWidget->textFocus();
    }

    return false;
}

FilterWidget* ProjectsWidget::filterWidget() const
{
    return m_filterWidget;
}

void ProjectsWidget::refreshProjectList()
{
    const int currentRow = m_listWidget->currentRow();
    m_listWidget->clear();

    if (UserManager::dir().isEmpty())
        return;

    auto projects = ProjectManager::projects();

    if (projects.size() < 1)
        return;

    for (auto hash : projects) {
        auto item = new ProjectListWidgetItem(this);
        item->setIcon(QIcon(PATH_FILEICON));
        item->setData(Hash, hash);
        item->setData(Name, ProjectManager::name(hash));
        item->setData(LastEdit, ProjectManager::toUiTime(ProjectManager::mfDate(hash)));
        item->setData(Active, hash == ProjectManager::hash());
        m_listWidget->addItem(item);
    }

    if (currentRow >= 0 && m_listWidget->count() > currentRow) {
        QListWidgetItem* currentItem = m_listWidget->item(currentRow);
        m_listWidget->sortItems();
        m_listWidget->setCurrentItem(currentItem);
    } else {
        m_listWidget->sortItems();
        m_listWidget->setCurrentRow(0);
    }
}

void ProjectsWidget::startProject()
{
    auto hash = m_listWidget->currentItem()->data(Hash).toString();

    if (!ProjectManager::start(hash)) {
        qWarning() << "Project starting unsuccessful.";
        refreshProjectList();
        return;
    }

    for (int i = m_listWidget->count(); i--;)
        m_listWidget->item(i)->setData(Active, false);

    m_listWidget->currentItem()->setData(Active, true);

    Delayer::delay([=] () -> bool {
        return m_progressBar->value() < m_progressBar->maximum();
    });
    Delayer::delay(200);

    unlock();
    emit done();
}

void ProjectsWidget::onNewButtonClick()
{
    if (UserManager::dir().isEmpty()) return;
    auto projects = ProjectManager::projectNames();
    int count = 1;
    QString projectName = "Project - 1";

    m_buttons->setDisabled(true);

    while (projects.contains(projectName)) {
        count++;
        projectName.remove(projectName.size() - 1, 1);
        projectName += QString::number(count);
    }

    auto item = new ProjectListWidgetItem(this);
    item->setIcon(QIcon(PATH_FILEICON));
    item->setData(Name, projectName);
    item->setData(LastEdit, ProjectManager::currentUiTime());
    item->setData(Active, false);
    m_listWidget->addItem(item);
    m_listWidget->setCurrentItem(item);

    Delayer::delay(250);

    m_buttons->setEnabled(true);

    emit newProject(projectName);
}

void ProjectsWidget::onLoadButtonClick()
{
    if (!m_listWidget->currentItem() || m_listWidget->currentItem()->isHidden()) {
        QMessageBox::warning(
                    this,
                    tr("Oops"),
                    tr("Select a project first.")
                    );
        return;
    }

    auto hash = m_listWidget->currentItem()->data(Hash).toString();
    auto chash = ProjectManager::hash();

    if (!chash.isEmpty() && chash == hash) {
        emit done();
        return;
    }

    // FIXME
    //    if (dW->qmlEditorView()->hasUnsavedDocs()) {
    //        QMessageBox msgBox;
    //        msgBox.setText(tr("%1 has some unsaved documents.").arg(ProjectManager::name()));
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

    WindowManager::mainWindow()->hide();
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

        if (!ProjectManager::exportProject(
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
            if (!ProjectManager::importProject(fileName)) {
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

void ProjectsWidget::onProgressChange(int progress)
{
    if (m_progressBar->indeterminate())
        m_progressBar->setIndeterminate(false);
    m_progressBar->setValue(progress);
}

void ProjectsWidget::onFilterTextChange(const QString& text)
{
    QListWidgetItem* firstVisibleItem = nullptr;
    for (int i = m_listWidget->count() - 1; i >= 0; --i) {
        QListWidgetItem* item = m_listWidget->item(i);
        const QString& projectName = item->data(Name).toString();
        item->setHidden(!text.isEmpty() && !projectName.contains(text, Qt::CaseInsensitive));
        if (!item->isHidden())
            firstVisibleItem = item;
    }
    if (firstVisibleItem)
        m_listWidget->setCurrentItem(firstVisibleItem);
    m_buttons_2->setVisible(firstVisibleItem);
}

void ProjectsWidget::lock()
{
    m_filterWidget->setDisabled(true);
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
    m_progressBar->setValue(0);
    m_progressBar->setIndeterminate(true);
}

void ProjectsWidget::unlock()
{
    m_filterWidget->setEnabled(true);
    m_buttons->setEnabled(true);
    m_listWidget->setEnabled(true);
    m_buttons_2->show();
    m_progressBar->hide();
}

#include "projectswidget.moc"