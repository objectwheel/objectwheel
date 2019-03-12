#include <projectdetailswidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <filemanager.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

#define BUTTONS_WIDTH    (338)
#define SIZE_ICON        (QSize(48, 48))
#define PATH_ICON        (":/images/options.png")
#define PATH_SICON       (":/images/load.png")
#define PATH_CICON       (":/images/unload.png")
#define PATH_DICON       (":/images/cancel.png")

enum Fields { Name, Description, Owner, CreationDate, ModificationDate, Size };
enum Buttons { Back, Save, Delete };

ProjectDetailsWidget::ProjectDetailsWidget(QWidget* parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_iconLabel = new QLabel;
    m_settingsLabel = new QLabel;
    m_bulkEdit = new BulkEdit;
    m_buttons = new ButtonSlice;

    m_layout->setSpacing(6);
    m_layout->addStretch();
    m_layout->addWidget(m_iconLabel,0 , Qt::AlignCenter);
    m_layout->addSpacing(6);
    m_layout->addWidget(m_settingsLabel,0 , Qt::AlignCenter);
    m_layout->addSpacing(5);
    m_layout->addWidget(m_bulkEdit, 0, Qt::AlignCenter);
    m_layout->addWidget(m_buttons, 0, Qt::AlignCenter);
    m_layout->addStretch();

    QPixmap p(PATH_ICON);
    p.setDevicePixelRatio(devicePixelRatioF());

    m_iconLabel->setFixedSize(SIZE_ICON);
    m_iconLabel->setPixmap(
        p.scaled(
            SIZE_ICON * devicePixelRatioF(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    m_settingsLabel->setFont(f);
    m_settingsLabel->setText(tr("Project Settings"));

    m_bulkEdit->add(Name, tr("Project Name"));
    m_bulkEdit->add(Description, tr("Description"));
    m_bulkEdit->add(Owner, tr("Owner"));
    m_bulkEdit->add(CreationDate, tr("Creation"));
    m_bulkEdit->add(ModificationDate, tr("Last Edit"));
    m_bulkEdit->add(Size, tr("Size"));
    m_bulkEdit->setFixedWidth(BUTTONS_WIDTH);

    static_cast<QLineEdit*>(m_bulkEdit->get(Name))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(m_bulkEdit->get(Description))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(m_bulkEdit->get(Owner))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(m_bulkEdit->get(CreationDate))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(m_bulkEdit->get(ModificationDate))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(m_bulkEdit->get(Size))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(m_bulkEdit->get(Owner))->setReadOnly(true);
    static_cast<QLineEdit*>(m_bulkEdit->get(CreationDate))->setReadOnly(true);
    static_cast<QLineEdit*>(m_bulkEdit->get(ModificationDate))->setReadOnly(true);
    static_cast<QLineEdit*>(m_bulkEdit->get(Size))->setReadOnly(true);

    static_cast<QLineEdit*>(m_bulkEdit->get(Owner))->setStyleSheet("color: #50000000; border: none; background: transparent;");
    static_cast<QLineEdit*>(m_bulkEdit->get(CreationDate))->setStyleSheet("color: #50000000; border: none; background: transparent;");
    static_cast<QLineEdit*>(m_bulkEdit->get(ModificationDate))->setStyleSheet("color: #50000000; border: none; background: transparent;");
    static_cast<QLineEdit*>(m_bulkEdit->get(Size))->setStyleSheet("color: #50000000; border: none; background: transparent;");

    m_buttons->add(Back, "#5BC5F8", "#2592F9");
    m_buttons->add(Delete, "#CC5D67", "#B2525A");
    m_buttons->add(Save, "#86CC63", "#75B257");
    m_buttons->get(Delete)->setText(tr("Delete"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Save)->setText(tr("Save"));
    m_buttons->get(Delete)->setIcon(QIcon(PATH_DICON));
    m_buttons->get(Back)->setIcon(QIcon(PATH_CICON));
    m_buttons->get(Save)->setIcon(QIcon(PATH_SICON));
    m_buttons->get(Delete)->setIconSize(QSize(16, 16));
    m_buttons->get(Back)->setIconSize(QSize(16, 16));
    m_buttons->get(Save)->setIconSize(QSize(16, 16));
    m_buttons->get(Delete)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Save)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = BUTTONS_WIDTH / 3.0;
    m_buttons->triggerSettings();

    connect(m_buttons->get(Save), &QPushButton::clicked, this, &ProjectDetailsWidget::onSaveClick);
    connect(m_buttons->get(Delete), &QPushButton::clicked, this, &ProjectDetailsWidget::onDeleteClick);
    connect(m_buttons->get(Back), &QPushButton::clicked, [=] {
        if (m_toTemplates)
            emit back();
        else
            emit done();
    });
}

void ProjectDetailsWidget::onEditProject(const QString& uid)
{
    m_toTemplates = false;
    m_uid = uid;
    ProjectManager::updateSize(m_uid);
    static_cast<QLineEdit*>(m_bulkEdit->get(Name))->setText(ProjectManager::name(uid));
    static_cast<QLineEdit*>(m_bulkEdit->get(Description))->setText(ProjectManager::description(uid));
    static_cast<QLineEdit*>(m_bulkEdit->get(Owner))->setText(ProjectManager::owner(uid));
    static_cast<QLineEdit*>(m_bulkEdit->get(CreationDate))->setText(ProjectManager::toUiTime(ProjectManager::crDate(uid)));
    static_cast<QLineEdit*>(m_bulkEdit->get(ModificationDate))->setText(ProjectManager::toUiTime(ProjectManager::mfDate(uid)));
    static_cast<QLineEdit*>(m_bulkEdit->get(Size))->setText(ProjectManager::size(uid));
}

void ProjectDetailsWidget::onNewProject(const QString& projectName, int templateNumber)
{
    m_toTemplates = true;
    m_templateNumber = templateNumber;
    m_uid.clear();
    static_cast<QLineEdit*>(m_bulkEdit->get(Name))->setText(projectName);
    static_cast<QLineEdit*>(m_bulkEdit->get(Description))->setText(tr("Simple project description."));
    static_cast<QLineEdit*>(m_bulkEdit->get(Owner))->setText(UserManager::user());
    static_cast<QLineEdit*>(m_bulkEdit->get(CreationDate))->setText(ProjectManager::currentUiTime());
    static_cast<QLineEdit*>(m_bulkEdit->get(ModificationDate))->setText(ProjectManager::currentUiTime());
    static_cast<QLineEdit*>(m_bulkEdit->get(Size))->setText(tr("0 bytes"));
}

void ProjectDetailsWidget::onSaveClick()
{
    auto projectnametext = static_cast<QLineEdit*>(m_bulkEdit->get(Name))->text();
    auto descriptiontext = static_cast<QLineEdit*>(m_bulkEdit->get(Description))->text();
    auto crdatetext = static_cast<QLineEdit*>(m_bulkEdit->get(CreationDate))->text();
    auto ownertext = static_cast<QLineEdit*>(m_bulkEdit->get(Owner))->text();

    if (projectnametext.isEmpty()) {
        QMessageBox::warning(this, tr("Oops"), tr("Project name cannot be empty."));
        return;
    }

    if (m_uid.isEmpty()) {
        if (!ProjectManager::newProject(
            m_templateNumber,
            projectnametext,
            descriptiontext,
            ownertext,
            ProjectManager::toDbTime(crdatetext)
        )) qFatal("ProjectDetailsWidget::onSaveClick() : Fatal Error. 0x01");
    } else {
        ProjectManager::instance()->changeName(
            m_uid,
            projectnametext
        );
        ProjectManager::instance()->changeDescription(
            m_uid,
            descriptiontext
        );
    }

    emit done();
}

void ProjectDetailsWidget::onDeleteClick()
{
    if (ProjectManager::dir(m_uid).isEmpty()) {
        emit done();
        return;
    }

    auto ret = QMessageBox::question(
        this,
        "Confirm Deletion",
        tr("You are about to delete %1 completely. Are you sure?").
        arg(ProjectManager::name(m_uid)),
        QMessageBox::Yes, QMessageBox::No | QMessageBox::Default
    );

    if (ret == QMessageBox::Yes) {
        const auto& cuid = ProjectManager::uid();
        if (!cuid.isEmpty() && cuid == m_uid)
            ProjectManager::stop();

        rm(ProjectManager::dir(m_uid));

        emit done();
    }
}
