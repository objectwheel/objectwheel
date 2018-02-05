#include <projectdetailswidget.h>
#include <fit.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <userbackend.h>

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>

#define BUTTONS_WIDTH    (fit::fx(350))
#define SIZE_ICON        (QSize(fit::fx(80), fit::fx(80)))
#define PATH_ICON        (":/resources/images/options.png")
#define PATH_SICON       (":/resources/images/load.png")
#define PATH_CICON       (":/resources/images/unload.png")
#define PATH_DICON       (":/resources/images/cancel.png")
#define pS               QApplication::primaryScreen()
#define TIME             QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate)

enum Fields { Name, Description, Owner, CreationDate, ModificationDate, Size };
enum Buttons { Cancel, Save, Delete };

ProjectDetailsWidget::ProjectDetailsWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _iconLabel = new QLabel;
    _settingsLabel = new QLabel;
    _bulkEdit = new BulkEdit;
    _buttons = new ButtonSlice;

    _layout->setSpacing(fit::fx(12));
    _layout->addStretch();
    _layout->addWidget(_iconLabel,0 , Qt::AlignCenter);
    _layout->addSpacing(fit::fx(15));
    _layout->addWidget(_settingsLabel,0 , Qt::AlignCenter);
    _layout->addSpacing(fit::fx(5));
    _layout->addWidget(_bulkEdit, 0, Qt::AlignCenter);
    _layout->addWidget(_buttons, 0, Qt::AlignCenter);
    _layout->addStretch();

    QPixmap p(PATH_ICON);
    p.setDevicePixelRatio(pS->devicePixelRatio());

    _iconLabel->setFixedSize(SIZE_ICON);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_ICON * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));

    _settingsLabel->setFont(f);
    _settingsLabel->setText(tr("Project Settings"));
    _settingsLabel->setStyleSheet("color: #2E3A41");

    _bulkEdit->add(Name, tr("Project Name"));
    _bulkEdit->add(Description, tr("Description"));
    _bulkEdit->add(Owner, tr("Owner"));
    _bulkEdit->add(CreationDate, tr("Creation"));
    _bulkEdit->add(ModificationDate, tr("Last Edit"));
    _bulkEdit->add(Size, tr("Size"));
    _bulkEdit->setFixedWidth(BUTTONS_WIDTH);

    static_cast<QLineEdit*>(_bulkEdit->get(Name))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Description))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Owner))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(CreationDate))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(ModificationDate))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Size))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Owner))->setReadOnly(true);
    static_cast<QLineEdit*>(_bulkEdit->get(CreationDate))->setReadOnly(true);
    static_cast<QLineEdit*>(_bulkEdit->get(ModificationDate))->setReadOnly(true);
    static_cast<QLineEdit*>(_bulkEdit->get(Size))->setReadOnly(true);

    static_cast<QLineEdit*>(_bulkEdit->get(Owner))->setStyleSheet("color: #3F4F59; border: none; background: transparent;");
    static_cast<QLineEdit*>(_bulkEdit->get(CreationDate))->setStyleSheet("color: #3F4F59; border: none; background: transparent;");
    static_cast<QLineEdit*>(_bulkEdit->get(ModificationDate))->setStyleSheet("color: #3F4F59; border: none; background: transparent;");
    static_cast<QLineEdit*>(_bulkEdit->get(Size))->setStyleSheet("color: #3F4F59; border: none; background: transparent;");

    _buttons->add(Delete, "#CC5D67", "#B2525A");
    _buttons->add(Cancel, "#5BC5F8", "#2592F9");
    _buttons->add(Save, "#86CC63", "#75B257");
    _buttons->get(Delete)->setText(tr("Delete Project"));
    _buttons->get(Cancel)->setText(tr("Cancel"));
    _buttons->get(Save)->setText(tr("Save"));
    _buttons->get(Delete)->setIcon(QIcon(PATH_DICON));
    _buttons->get(Cancel)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Save)->setIcon(QIcon(PATH_SICON));
    _buttons->get(Delete)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Save)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 3.0;
    _buttons->triggerSettings();

    connect(_buttons->get(Save), SIGNAL(clicked(bool)), SLOT(onSaveClick()));
    connect(_buttons->get(Delete), SIGNAL(clicked(bool)), SLOT(onDeleteClick()));
    connect(_buttons->get(Cancel), SIGNAL(clicked(bool)), SIGNAL(done()));
}

void ProjectDetailsWidget::onNewProject(const QString& projectName)
{
    static_cast<QLineEdit*>(_bulkEdit->get(Name))->setText(projectName);
    static_cast<QLineEdit*>(_bulkEdit->get(Description))->setText(tr("Simple project description."));
    static_cast<QLineEdit*>(_bulkEdit->get(Owner))->setText(UserBackend::instance()->user() + " •");
    static_cast<QLineEdit*>(_bulkEdit->get(CreationDate))->setText(TIME + " •");
    static_cast<QLineEdit*>(_bulkEdit->get(ModificationDate))->setText(TIME + " •");
    static_cast<QLineEdit*>(_bulkEdit->get(Size))->setText(tr("0 bytes") + " •");

}

void ProjectDetailsWidget::onSaveClick()
{

}

void ProjectDetailsWidget::onDeleteClick()
{

}
