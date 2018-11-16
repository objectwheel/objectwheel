#include <behaviorsettingswidget.h>
#include <codeeditorsettings.h>
#include <behaviorsettings.h>

#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>

BehaviorSettingsWidget::BehaviorSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_savingGroup(new QGroupBox(contentWidget()))
  , m_savingLayout(new QVBoxLayout(m_savingGroup))
  , m_autoSaveAfterRunningBox(new QCheckBox(m_savingGroup))
{
    contentLayout()->addWidget(m_savingGroup);
    contentLayout()->addStretch();

    /****/

    m_savingLayout->setSpacing(8);
    m_savingLayout->setContentsMargins(6, 6, 6, 6);
    m_savingLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_savingLayout->addWidget(m_autoSaveAfterRunningBox);

    m_savingGroup->setTitle(tr("Saving"));
    m_autoSaveAfterRunningBox->setText(tr("Auto save code changes before user runs a project"));

    m_autoSaveAfterRunningBox->setToolTip(tr("Enable automatic code saving right before user runs a project"));

    m_autoSaveAfterRunningBox->setCursor(Qt::PointingHandCursor);

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
    settings->autoSaveBeforeRunning  = m_autoSaveAfterRunningBox->isChecked();
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
    m_autoSaveAfterRunningBox->setChecked(settings->autoSaveBeforeRunning);
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
            || m_autoSaveAfterRunningBox->text().contains(word, Qt::CaseInsensitive);
}
