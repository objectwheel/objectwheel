#include <projectdetailswidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>

#define BUTTONS_WIDTH    (338)
#define PATH_SICON       (":/images/welcome/load.png")
#define PATH_CICON       (":/images/welcome/unload.png")
#define PATH_DICON       (":/images/welcome/cancel.png")

enum Fields { Name, Description, CreationDate, ModificationDate, Size };
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

    m_iconLabel->setFixedSize(QSize(60, 60));
    m_iconLabel->setPixmap(PaintUtils::pixmap(":/images/welcome/settings.svg", QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_settingsLabel->setFont(f);
    m_settingsLabel->setText(tr("Project Settings"));

    m_bulkEdit->add(Name, tr("Project Name"));
    m_bulkEdit->add(Description, tr("Description"));
    m_bulkEdit->add(CreationDate, tr("Creation"));
    m_bulkEdit->add(ModificationDate, tr("Last Edit"));
    m_bulkEdit->add(Size, tr("Size"));
    m_bulkEdit->setFixedWidth(BUTTONS_WIDTH);

    m_bulkEdit->get<QLineEdit*>(Name)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Description)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CreationDate)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(ModificationDate)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Size)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CreationDate)->setReadOnly(true);
    m_bulkEdit->get<QLineEdit*>(ModificationDate)->setReadOnly(true);
    m_bulkEdit->get<QLineEdit*>(Size)->setReadOnly(true);

    m_bulkEdit->get<QLineEdit*>(CreationDate)->setStyleSheet("color: #50000000; border: none; background: transparent;");
    m_bulkEdit->get<QLineEdit*>(ModificationDate)->setStyleSheet("color: #50000000; border: none; background: transparent;");
    m_bulkEdit->get<QLineEdit*>(Size)->setStyleSheet("color: #50000000; border: none; background: transparent;");

    m_buttons->add(Back, "#5BC5F8", "#2592F9");
    m_buttons->add(Delete, "#CC5D67", "#B2525A");
    m_buttons->add(Save, "#86CC63", "#75B257");
    m_buttons->get(Delete)->setText(tr("Delete"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Save)->setText(tr("Save"));
    m_buttons->get(Delete)->setIcon(QIcon(PATH_DICON));
    m_buttons->get(Back)->setIcon(QIcon(PATH_CICON));
    m_buttons->get(Save)->setIcon(QIcon(PATH_SICON));
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
    m_bulkEdit->get<QLineEdit*>(Name)->setText(ProjectManager::name(uid));
    m_bulkEdit->get<QLineEdit*>(Description)->setText(ProjectManager::description(uid));
    m_bulkEdit->get<QLineEdit*>(CreationDate)->setText(ProjectManager::crDate(uid).toString(Qt::SystemLocaleLongDate));
    m_bulkEdit->get<QLineEdit*>(ModificationDate)->setText(ProjectManager::mfDate(uid).toString(Qt::SystemLocaleLongDate));
    m_bulkEdit->get<QLineEdit*>(Size)->setText(UtilityFunctions::toPrettyBytesString(ProjectManager::size(uid)));
}

void ProjectDetailsWidget::onNewProject(const QString& projectName, int templateNumber)
{
    m_toTemplates = true;
    m_templateNumber = templateNumber;
    m_uid.clear();
    m_bulkEdit->get<QLineEdit*>(Name)->setText(projectName);
    m_bulkEdit->get<QLineEdit*>(Description)->setText(tr("Simple project description."));
    m_bulkEdit->get<QLineEdit*>(CreationDate)->setText(QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate));
    m_bulkEdit->get<QLineEdit*>(ModificationDate)->setText(QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate));
    m_bulkEdit->get<QLineEdit*>(Size)->setText(tr("0 bytes"));
}

void ProjectDetailsWidget::onSaveClick()
{
    auto projectnametext = m_bulkEdit->get<QLineEdit*>(Name)->text();
    auto descriptiontext = m_bulkEdit->get<QLineEdit*>(Description)->text();
    auto crdatetext = m_bulkEdit->get<QLineEdit*>(CreationDate)->text();

    if (projectnametext.isEmpty()) {
        UtilityFunctions::showMessage(this, tr("Oops"), tr("The project name cannot be empty."));
        return;
    }

    if (m_uid.isEmpty()) {
        if (!ProjectManager::initProject(
            m_templateNumber,
            projectnametext,
            descriptiontext,
            QDateTime::fromString(crdatetext, Qt::SystemLocaleLongDate)
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

    int ret = UtilityFunctions::showMessage(this, tr("Confirm deletion"),
                                            tr("You are about to delete %1 completely. Are you sure?").
                                            arg(ProjectManager::name(m_uid)), QMessageBox::Question,
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        const auto& cuid = ProjectManager::uid();
        if (!cuid.isEmpty() && cuid == m_uid)
            ProjectManager::stop();

        QDir(ProjectManager::dir(m_uid)).removeRecursively();

        emit done();
    }
}
