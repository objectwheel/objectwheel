#include <projectswidget.h>
#include <buttonslice.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <delayer.h>
#include <controlrenderingmanager.h>
#include <progressbar.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <utilityfunctions.h>
#include <transparentstyle.h>
#include <planmanager.h>

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
enum Roles { Name = Qt::UserRole + 1, LastEdit, Uid, Active };

namespace {

QListWidgetItem* itemForUid(const QListWidget* listWidget, const QString& uid)
{
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if (item->data(Uid).toString() == uid)
            return item;
    }
    return nullptr;
}
}

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget* parent = nullptr) : QWidget(parent)
      , m_layout(new QHBoxLayout(this))
      , m_sortLabel(new QLabel)
      , m_sortComboBox(new QComboBox)
      , m_searchLineEdit(new QLineEdit)
      , m_reverseSortCheckBox(new QCheckBox)
    {
        m_layout->setContentsMargins(2, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(m_searchLineEdit);
        m_layout->addWidget(m_reverseSortCheckBox);
        m_layout->addWidget(m_sortLabel);
        m_layout->addWidget(m_sortComboBox);

        TransparentStyle::attach(this);

        m_sortLabel->setText(" " + tr("Reverse by") + " ");

        m_searchLineEdit->setPlaceholderText(tr("Search"));
        m_searchLineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
        m_searchLineEdit->setClearButtonEnabled(true);
        m_searchLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        m_searchLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_searchLineEdit->setToolTip(tr("Search by name"));

        m_sortComboBox->addItem(tr("Date"));
        m_sortComboBox->addItem(tr("Name"));
        m_sortComboBox->setCursor(Qt::PointingHandCursor);
        m_sortComboBox->setToolTip(tr("Change sorting criteria"));
        m_sortComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        m_sortComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        m_reverseSortCheckBox->setToolTip(tr("Activate reverse sort"));
        m_reverseSortCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_reverseSortCheckBox->setCursor(Qt::PointingHandCursor);

        connect(m_searchLineEdit, &QLineEdit::textChanged, this, &SearchWidget::searchTextChanged);
        connect(m_sortComboBox, &QComboBox::currentTextChanged, this, &SearchWidget::sortCriteriaChanged);
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
        m_searchLineEdit->setFocus();
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
    void searchTextChanged(const QString& text);
    void sortCriteriaChanged(const QString& criteria);

private:
    QHBoxLayout* m_layout;
    QLabel* m_sortLabel;
    QComboBox* m_sortComboBox;
    QLineEdit* m_searchLineEdit;
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
        Q_ASSERT(UtilityFunctions::window(m_listWidget));
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
        const SearchWidget* searchWidget = m_projectsWidget->searchWidget();
        const QString& criteria = searchWidget->sortingCriteria();
        bool reverseSort = searchWidget->isReverseSort();

        if (criteria == QObject::tr("Name")) {
            const QString& myName = data(Name).toString();
            const QString& othersName = other.data(Name).toString();
            bool result = QString::localeAwareCompare(myName, othersName) < 0;
            return reverseSort ? !result : result;
        }
        if (criteria == QObject::tr("Date")) {
            const QDateTime& myDate = QDateTime::fromString(data(LastEdit).toString(), Qt::SystemLocaleLongDate);
            const QDateTime& othersDate = QDateTime::fromString(other.data(LastEdit).toString(), Qt::SystemLocaleLongDate);
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
  , m_searchWidget(new SearchWidget)
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
    m_layout->addWidget(m_searchWidget, 0, Qt::AlignCenter);
    m_layout->addWidget(m_listWidget, 0, Qt::AlignCenter);
    m_layout->addWidget(m_buttons, 0, Qt::AlignCenter);
    m_layout->addStretch();

    // Since viewport (parent of the m_progressBar) has its
    // own layout and we don't add m_progressBar into it
    // QWidget::setVisible does not adjust the size. So we
    // must call it manually.
    m_progressBar->adjustSize();
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
    m_welcomeLabel->setText(tr("Welcome to ") + APP_NAME);

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_versionLabel->setFont(f);
    m_versionLabel->setText(tr("Version ") + tr(APP_VER) + " (" APP_GITHASH ")");

    m_projectsLabel->setText(tr("Your Projects"));

    m_searchWidget->setFixedWidth(SIZE_LIST.width());
    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &ProjectsWidget::onSearchTextChange);
    connect(m_searchWidget, &SearchWidget::sortCriteriaChanged, [=] {
        m_listWidget->sortItems();
        m_listWidget->scrollToItem(m_listWidget->currentItem());
        updateGadgetPositions();
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

    m_buttons_2->setFixedSize(20, 20);
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
            updateGadgetPositions();
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

    connect(m_buttons->get(New), &QPushButton::clicked,
            this, &ProjectsWidget::onNewButtonClick);
    connect(m_buttons->get(Load), &QPushButton::clicked,
            this, &ProjectsWidget::onLoadButtonClick);
    connect(m_buttons->get(Import), &QPushButton::clicked,
            this, &ProjectsWidget::onImportButtonClick);
    connect(m_buttons->get(Export), &QPushButton::clicked,
            this, &ProjectsWidget::onExportButtonClick);
    connect(m_buttons_2->get(Settings), &QPushButton::clicked,
            this, &ProjectsWidget::onSettingsButtonClick);

    connect(ControlRenderingManager::instance(), &ControlRenderingManager::initializationProgressChanged,
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
            m_searchWidget->textFocus();
    }

    return false;
}

SearchWidget* ProjectsWidget::searchWidget() const
{
    return m_searchWidget;
}

// FIXME: This function has severe performance issues.
void ProjectsWidget::refreshProjectList(bool selectionPreserved)
{
    m_listWidget->clear();

    if (!UserManager::isLoggedIn())
        return;

    int row = 0;
    if (m_listWidget->currentItem())
        row = m_listWidget->currentRow();

    auto projects = ProjectManager::projects();

    if (projects.size() < 1)
        return;

    for (auto uid : projects) {
        auto item = new ProjectListWidgetItem(this);
        item->setIcon(QIcon(PATH_FILEICON));
        item->setData(Uid, uid);
        item->setData(Name, ProjectManager::name(uid));
        item->setData(LastEdit, ProjectManager::mfDate(uid).toString(Qt::SystemLocaleLongDate));
        item->setData(Active, uid == ProjectManager::uid());
        m_listWidget->addItem(item);
    }

    if (!projects.isEmpty()) {
        m_listWidget->sortItems();
        if (selectionPreserved) {
            m_listWidget->setCurrentRow(row);
            if (const QListWidgetItem* item = m_listWidget->item(row))
                m_listWidget->scrollToItem(item);
        } else {
            m_listWidget->setCurrentRow(0);
        }
    }
}

void ProjectsWidget::onNewButtonClick()
{
    if (!UserManager::isLoggedIn())
        return;

    auto projects = ProjectManager::projectNames();

    if (!PlanManager::isEligibleForNewProject(UserManager::plan(), projects.size())) {
        UtilityFunctions::showMessage(this,
                                      tr("You are not eligible for this"),
                                      tr("Please upgrade your plan in order to have "
                                         "more than 3 projects at the same time."));
        return;
    }

    m_buttons->setDisabled(true);
    QString projectName = tr("Project") + " - 1";

    while (projects.contains(projectName))
        projectName = UtilityFunctions::increasedNumberedText(projectName, true, true);

    auto item = new ProjectListWidgetItem(this);
    item->setIcon(QIcon(PATH_FILEICON));
    item->setData(Name, projectName);
    item->setData(LastEdit, QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate));
    item->setData(Active, false);
    m_listWidget->addItem(item);
    m_listWidget->sortItems();
    m_listWidget->setCurrentItem(item);
    m_listWidget->scrollToItem(item);

    Delayer::delay(250);

    m_buttons->setEnabled(true);

    emit newProject(projectName);
}

void ProjectsWidget::onLoadButtonClick()
{
    if (!m_listWidget->currentItem() || m_listWidget->currentItem()->isHidden()) {
        UtilityFunctions::showMessage(this, tr("Oops"), tr("Select a project first."));
        return;
    }

    auto uid = m_listWidget->currentItem()->data(Uid).toString();
    auto cuid = ProjectManager::uid();

    if (!cuid.isEmpty() && cuid == uid) {
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

    //! Start sequance
    lock();

    if (!ProjectManager::start(uid)) {
        UtilityFunctions::showMessage(this, tr("Oops"), tr("Project start unsuccessful."),
                                      QMessageBox::Critical);
        refreshProjectList();
        QListWidgetItem* item = itemForUid(m_listWidget, uid);
        if (item) {
            m_listWidget->setCurrentItem(item);
            m_listWidget->scrollToItem(item);
        }
        return;
    }

    for (int i = m_listWidget->count(); i--;)
        m_listWidget->item(i)->setData(Active, false);

    m_listWidget->currentItem()->setData(Active, true);
    m_listWidget->currentItem()->setData(LastEdit, QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate));

    Delayer::delay([=] () -> bool {
        return m_progressBar->value() < m_progressBar->maximum();
    });
    Delayer::delay(200);

    m_listWidget->sortItems();
    m_listWidget->scrollToItem(m_listWidget->currentItem());
    updateGadgetPositions();

    unlock();
    emit done();
}

void ProjectsWidget::onExportButtonClick()
{
    if (!m_listWidget->currentItem()) {
        UtilityFunctions::showMessage(this, tr("Oops"), tr("Select the project first."));
        return;
    }

    auto uid = m_listWidget->currentItem()->data(Uid).toString();
    auto pname = m_listWidget->currentItem()->data(Name).toString();

    if (uid.isEmpty() || pname.isEmpty())
        return;

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec()) {
        if (!ProjectManager::exportProject(uid, dialog.selectedFiles().at(0) +
                                           '/' + pname + ".opf")) {
            return;
        }

        UtilityFunctions::showMessage(this, tr("Done"),
                                      tr("Project export has successfully finished."),
                                      QMessageBox::Information);
    }
}

void ProjectsWidget::onImportButtonClick()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Objectwheel Projects (*.opf)"));
    dialog.setViewMode(QFileDialog::Detail);

    if (dialog.exec()) {
        QString uid;
        for (const QString& fileName : dialog.selectedFiles()) {
            if (!ProjectManager::importProject(fileName, &uid)) {
                UtilityFunctions::showMessage(this, tr("Operation stopped"),
                                              tr("One or more import files broken."));
                return;
            }
        }
        Q_ASSERT(!uid.isEmpty());

        refreshProjectList();
        QListWidgetItem* item = itemForUid(m_listWidget, uid);
        if (item) {
            m_listWidget->setCurrentItem(item);
            m_listWidget->scrollToItem(item);
        }
        UtilityFunctions::showMessage(this, tr("Done"),
                                      tr("Project import has successfully finished."),
                                      QMessageBox::Information);
    }
}

void ProjectsWidget::onSettingsButtonClick()
{
    if (m_listWidget->currentItem())
        emit editProject(m_listWidget->currentItem()->data(Uid).toString());
}

void ProjectsWidget::onProgressChange(int progress)
{
    if (m_progressBar->isIndeterminate())
        m_progressBar->setIndeterminate(false);
    m_progressBar->setValue(progress);
}

void ProjectsWidget::onSearchTextChange(const QString& text)
{
    QListWidgetItem* firstVisibleItem = nullptr;
    for (int i = m_listWidget->count() - 1; i >= 0; --i) {
        QListWidgetItem* item = m_listWidget->item(i);
        const QString& projectName = item->data(Name).toString();
        item->setHidden(!text.isEmpty() && !projectName.contains(text, Qt::CaseInsensitive));
        if (!item->isHidden())
            firstVisibleItem = item;
    }
    if (firstVisibleItem) {
        m_listWidget->setCurrentItem(firstVisibleItem);
        m_listWidget->scrollToItem(firstVisibleItem);
    }
    m_buttons_2->setVisible(firstVisibleItem);
}

void ProjectsWidget::lock()
{
    m_searchWidget->setDisabled(true);
    m_buttons->setDisabled(true);
    m_listWidget->setDisabled(true);
    m_buttons_2->hide();

    m_progressBar->setIndeterminate(true);
    m_progressBar->show();
    m_progressBar->raise();
    m_progressBar->setValue(0);
}

void ProjectsWidget::unlock()
{
    m_searchWidget->setEnabled(true);
    m_buttons->setEnabled(true);
    m_listWidget->setEnabled(true);
    m_buttons_2->show();
    m_progressBar->hide();
}

void ProjectsWidget::updateGadgetPositions()
{
    if (!m_listWidget->currentItem())
        return;

    const QRect& rect = m_listWidget->visualItemRect(m_listWidget->currentItem());
    m_buttons_2->move(rect.topRight().x() - m_buttons_2->width() - 5,
                      rect.topRight().y() + (rect.height() - m_buttons_2->height()) / 2.0);
    m_progressBar->move(m_buttons_2->pos() + QPoint(-WIDTH_PROGRESS + m_buttons_2->width(),
                                                    m_buttons_2->height() / 2.0 -
                                                    m_progressBar->height() / 2.0));
}

#include "projectswidget.moc"
