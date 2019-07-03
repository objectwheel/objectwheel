#include <behaviorsettingswidget.h>
#include <codeeditorsettings.h>
#include <behaviorsettings.h>

#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>

BehaviorSettingsWidget::BehaviorSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_savingGroup(new QGroupBox(contentWidget()))
  , m_autoSaveBeforeRunningBox(new QCheckBox(m_savingGroup))
{
    contentLayout()->addWidget(m_savingGroup);
    contentLayout()->addStretch();

    /****/

    auto savingLayout = new QVBoxLayout(m_savingGroup);
    savingLayout->setSpacing(8);
    savingLayout->setContentsMargins(6, 6, 6, 6);
    savingLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    savingLayout->addWidget(m_autoSaveBeforeRunningBox);

    m_savingGroup->setTitle(tr("Saving"));
    m_autoSaveBeforeRunningBox->setText(tr("Automatically save changes on the code before running any project."));

    m_autoSaveBeforeRunningBox->setToolTip(tr("Enable automatic code saving before running a project"));

    m_autoSaveBeforeRunningBox->setCursor(Qt::PointingHandCursor);

    /****/

    activate();
    reset();
}

void BehaviorSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    BehaviorSettings* settings = CodeEditorSettings::behaviorSettings();
    /****/
    settings->autoSaveBeforeRunning  = m_autoSaveBeforeRunningBox->isChecked();
    /****/
    settings->write();
}

void BehaviorSettingsWidget::reset()
{
    if (!isActivated())
        return;

    activate(false);

    const BehaviorSettings* settings = CodeEditorSettings::behaviorSettings();
    /****/
    m_autoSaveBeforeRunningBox->setChecked(settings->autoSaveBeforeRunning);
}

QIcon BehaviorSettingsWidget::icon() const
{
    return QIcon(":/images/settings/behavior.png");
}

QString BehaviorSettingsWidget::title() const
{
    return tr("Behavior");
}

bool BehaviorSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_savingGroup->title().contains(word, Qt::CaseInsensitive)
            || m_autoSaveBeforeRunningBox->text().contains(word, Qt::CaseInsensitive);
}
