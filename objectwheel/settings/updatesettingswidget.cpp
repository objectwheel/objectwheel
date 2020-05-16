#include <updatesettingswidget.h>
#include <updatesettings.h>
#include <generalsettings.h>
#include <paintutils.h>
#include <servermanager.h>
#include <updatemanager.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>
#include <QStackedLayout>

UpdateSettingsWidget::UpdateSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_currentVersionGroup(new QGroupBox(contentWidget()))
  , m_logoLabel(new QLabel(m_currentVersionGroup))
  , m_brandIconLabel(new QLabel(m_currentVersionGroup))
  , m_versionLabel(new QLabel(m_currentVersionGroup))
  , m_revisionLabel(new QLabel(m_currentVersionGroup))
  , m_buildDateLabel(new QLabel(m_currentVersionGroup))
  /****/
  , m_updateGroup(new QGroupBox(contentWidget()))
  , m_checkForUpdatesAutomaticallyCheckBox(new QCheckBox(m_updateGroup))
  , m_checkUpdatesButton(new QPushButton(m_updateGroup))
  , m_updateStatusStackedLayout(new QStackedLayout)
  /*__*/
  , m_upToDateWidget(new QWidget(m_updateGroup))
  , m_upToDateLabel(new QLabel(m_upToDateWidget))
  , m_lastCheckedLabel(new QLabel(m_upToDateWidget))
  , m_lastCheckedDateLabel(new QLabel(m_upToDateWidget))
  , m_upToDateIcon(new QLabel(m_upToDateWidget))
{
    contentLayout()->addWidget(m_currentVersionGroup);
    contentLayout()->addWidget(m_updateGroup);
    contentLayout()->addStretch();

    /****/

    auto currentVersionLayout = new QGridLayout(m_currentVersionGroup);
    currentVersionLayout->setSpacing(6);
    currentVersionLayout->setContentsMargins(4, 4, 4, 4);
    currentVersionLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    currentVersionLayout->addWidget(m_logoLabel, 0, 0, 6 , 1, Qt::AlignLeft | Qt::AlignVCenter);
    currentVersionLayout->addWidget(m_brandIconLabel, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentVersionLayout->addWidget(m_versionLabel, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentVersionLayout->addWidget(m_revisionLabel, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentVersionLayout->addWidget(m_buildDateLabel, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentVersionLayout->setRowStretch(0, 1);
    currentVersionLayout->setRowStretch(5, 1);
    currentVersionLayout->setColumnStretch(3, 1);
    currentVersionLayout->setColumnMinimumWidth(1, currentVersionLayout->contentsMargins().left());

    m_currentVersionGroup->setTitle(tr("Current Version"));
    m_brandIconLabel->setText(QLatin1String("<span style=\"font-weight:500;\">Objectwheel (Beta)</span>"));
    m_versionLabel->setText(tr("Version: ") + QStringLiteral(APP_VER));
    m_revisionLabel->setText(tr("Revision: ") + QStringLiteral(APP_GITHASH));
    m_buildDateLabel->setText(tr("Build Date: ") + QStringLiteral(APP_GITDATE));

    m_logoLabel->setFixedSize(QSize(96, 96));
    m_logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/icon.png"), QSize(96, 96), this));

    /****/

    auto updateLayout = new QGridLayout(m_updateGroup);
    updateLayout->setSpacing(6);
    updateLayout->setContentsMargins(4, 4, 4, 4);
    updateLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    updateLayout->addLayout(m_updateStatusStackedLayout, 0, 0, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    updateLayout->addWidget(m_checkForUpdatesAutomaticallyCheckBox, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    updateLayout->addWidget(m_checkUpdatesButton, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    updateLayout->setColumnStretch(2, 1);

    m_updateGroup->setTitle(tr("Update"));
    m_checkForUpdatesAutomaticallyCheckBox->setText(tr("Check for updates automatically"));
    m_checkUpdatesButton->setText(tr("Check Now"));

    m_checkForUpdatesAutomaticallyCheckBox->setCursor(Qt::PointingHandCursor);
    m_checkUpdatesButton->setCursor(Qt::PointingHandCursor);

    m_updateStatusStackedLayout->addWidget(m_upToDateWidget);

    /*__*/

    auto upToDateLayout = new QGridLayout(m_upToDateWidget);
    upToDateLayout->setSpacing(6);
    upToDateLayout->setContentsMargins(4, 4, 4, 4);
    upToDateLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    upToDateLayout->addWidget(m_upToDateIcon, 0, 0, 3, 1, Qt::AlignLeft | Qt::AlignTop);
    upToDateLayout->addWidget(m_upToDateLabel, 0, 2, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    upToDateLayout->addWidget(m_lastCheckedLabel, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    upToDateLayout->addWidget(m_lastCheckedDateLabel, 1, 3, Qt::AlignLeft | Qt::AlignVCenter);
    upToDateLayout->setRowStretch(2, 1);
    upToDateLayout->setColumnMinimumWidth(1, upToDateLayout->contentsMargins().left());

    m_upToDateLabel->setText(QLatin1String("<span style=\"font-weight:500;\">%1</span>")
                             .arg(tr("Objectwheel is up to date")));
    m_lastCheckedLabel->setText(tr("Last checked:"));

    m_upToDateIcon->setFixedSize(QSize(96, 96));
    m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/run/computer.svg"), QSize(96, 96), this));

    /****/

    QObject::connect(ServerManager::instance(), &ServerManager::connected,
                     this, [] {
        // TODO: May we check for updates every 24 hours or something
        if (GeneralSettings::updateSettings()->checkForUpdatesAutomatically
                && UpdateManager::remoteMetaInfo().isEmpty()) {
            UpdateManager::scheduleUpdateCheck();
        }
    }, Qt::QueuedConnection);
    QObject::connect(UpdateManager::instance(), &UpdateManager::updateCheckFinished,
                     this, [] {
        // TODO
    }, Qt::QueuedConnection);

    /****/

}

void UpdateSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);
}

void UpdateSettingsWidget::revert()
{
    if (!isActivated())
        return;

    activate(false);
}

void UpdateSettingsWidget::reset()
{
    GeneralSettings::updateSettings()->reset();
    GeneralSettings::updateSettings()->write();
    activate();
    revert();
    emit GeneralSettings::instance()->designerStateReset();
}

QIcon UpdateSettingsWidget::icon() const
{
    return QIcon(":/images/settings/update.svg");
}

QString UpdateSettingsWidget::title() const
{
    return tr("Update");
}

bool UpdateSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive);
}

void UpdateSettingsWidget::fill()
{
}