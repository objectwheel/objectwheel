#include <updatesettingswidget.h>
#include <updatesettings.h>
#include <systemsettings.h>
#include <paintutils.h>
#include <servermanager.h>
#include <updatemanager.h>
#include <stackedlayout.h>

#include <QCoreApplication>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>
#include <QTextEdit>

Q_DECLARE_METATYPE(QMargins)

const char layoutMarginsProperty[] = "_q_ApplicationStyle_layoutMarginsProperty";

UpdateSettingsWidget::UpdateSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_updateGroup(new QGroupBox(contentWidget()))
  , m_updateStatusStackedLayout(new StackedLayout(m_updateGroup))
  /*__*/
  , m_upToDateWidget(new QWidget(m_updateGroup))
  , m_upToDateLabel(new QLabel(m_upToDateWidget))
  , m_lastCheckedLabel(new QLabel(m_upToDateWidget))
  , m_lastCheckedDateLabel(new QLabel(m_upToDateWidget))
  , m_upToDateIcon(new QLabel(m_upToDateWidget))
  , m_checkUpdatesButton(new QPushButton(m_upToDateWidget))
  /*__*/
  , m_updatesAvailableWidget(new QWidget(m_updateGroup))
  , m_updatesAvailableLabel(new QLabel(m_updatesAvailableWidget))
  , m_updatesAvailableIcon(new QLabel(m_updatesAvailableWidget))
  , m_changelogEdit(new QTextEdit(m_updatesAvailableWidget))
  , m_updateButton(new QPushButton(m_updatesAvailableWidget))
  /****/
  , m_statusGroup(new QGroupBox(contentWidget()))
  , m_logoLabel(new QLabel(m_statusGroup))
  , m_brandIconLabel(new QLabel(m_statusGroup))
  , m_versionLabel(new QLabel(m_statusGroup))
  , m_revisionLabel(new QLabel(m_statusGroup))
  , m_buildDateLabel(new QLabel(m_statusGroup))
  /****/
  , m_settingsGroup(new QGroupBox(contentWidget()))
  , m_checkForUpdatesAutomaticallyCheckBox(new QCheckBox(m_settingsGroup))
{
    contentLayout()->addWidget(m_updateGroup);
    contentLayout()->addWidget(m_statusGroup);
    contentLayout()->addWidget(m_settingsGroup);
    contentLayout()->addStretch();

    /****/

    m_updateStatusStackedLayout->setSpacing(6);
    m_updateStatusStackedLayout->setContentsMargins(6, 6, 6, 6);
    m_updateStatusStackedLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_updateStatusStackedLayout->addWidget(m_upToDateWidget);
    m_updateStatusStackedLayout->addWidget(m_updatesAvailableWidget);

    m_updateGroup->setTitle(tr("Update"));

    /*__*/

    auto upToDateLayout = new QGridLayout(m_upToDateWidget);
    upToDateLayout->setSpacing(6);
    upToDateLayout->setContentsMargins(6, 6, 6, 6);
    upToDateLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    upToDateLayout->addWidget(m_upToDateIcon, 0, 0, 4, 1, Qt::AlignTop);
    upToDateLayout->addWidget(m_upToDateLabel, 0, 1, 1, 2);
    upToDateLayout->addWidget(m_lastCheckedLabel, 1, 1);
    upToDateLayout->addWidget(m_lastCheckedDateLabel, 1, 2);
    upToDateLayout->addWidget(m_checkUpdatesButton, 2, 1);
    upToDateLayout->setRowStretch(3, 1);

    m_checkUpdatesButton->setText(tr("Check Now"));
    m_lastCheckedLabel->setText(tr("Last checked:"));

    m_upToDateIcon->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, -4, 0, 0)));
    m_upToDateIcon->setFixedSize(QSize(80, 80));
    m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/up-to-date.svg"), QSize(80, 80), this));
    QCoreApplication::postEvent(m_upToDateIcon, new QEvent(QEvent::StyleChange)); // Apply margin change

    m_checkUpdatesButton->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(-1, 0, 0, 0)));
    m_checkUpdatesButton->setCursor(Qt::PointingHandCursor);
    m_checkUpdatesButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QCoreApplication::postEvent(m_checkUpdatesButton, new QEvent(QEvent::StyleChange)); // Apply margin change

    /*__*/

    auto updatesAvailableLayout = new QGridLayout(m_updatesAvailableWidget);
    updatesAvailableLayout->setSpacing(6);
    updatesAvailableLayout->setContentsMargins(6, 6, 6, 6);
    updatesAvailableLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    updatesAvailableLayout->addWidget(m_updateButton, 0, 0);
    updatesAvailableLayout->addWidget(m_updatesAvailableLabel, 0, 1);
    updatesAvailableLayout->addWidget(m_updatesAvailableIcon, 1, 0, Qt::AlignTop);
    updatesAvailableLayout->addWidget(m_changelogEdit, 1, 1);

    m_updateButton->setText(tr("Update"));

    m_updatesAvailableIcon->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, -7, 0, 0)));
    m_updatesAvailableIcon->setFixedSize(QSize(80, 80));
    m_updatesAvailableIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/updates-available.svg"), QSize(80, 80), this));
    QCoreApplication::postEvent(m_updatesAvailableIcon, new QEvent(QEvent::StyleChange)); // Apply margin change

    m_changelogEdit->setReadOnly(true);
    UtilityFunctions::adjustFontPixelSize(m_changelogEdit, -1);
    m_changelogEdit->setMinimumHeight(170);

    m_updateButton->setCursor(Qt::PointingHandCursor);
    m_updateButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    /****/

    auto statusLayout = new QGridLayout(m_statusGroup);
    statusLayout->setSpacing(6);
    statusLayout->setContentsMargins(6, 6, 6, 6);
    statusLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    statusLayout->addWidget(m_logoLabel, 0, 0, 4, 1, Qt::AlignVCenter);
    statusLayout->addWidget(m_brandIconLabel, 0, 1);
    statusLayout->addWidget(m_versionLabel, 1, 1);
    statusLayout->addWidget(m_revisionLabel, 2, 1);
    statusLayout->addWidget(m_buildDateLabel, 3, 1);

    m_statusGroup->setTitle(tr("Status"));
    m_brandIconLabel->setText(QLatin1String("Objectwheel (Beta)"));
    m_versionLabel->setText(tr("Version: ") + QStringLiteral(APP_VER));
    m_revisionLabel->setText(tr("Revision: ") + QStringLiteral(APP_GITHASH));
    m_buildDateLabel->setText(tr("Build date: ") + QDateTime::fromString(APP_GITDATE, Qt::ISODate).toString(Qt::SystemLocaleLongDate));

    m_logoLabel->setFixedSize(QSize(80, 80));
    m_logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/icon.png"), QSize(80, 80), this));

    /****/

    auto settingsLayout = new QVBoxLayout(m_settingsGroup);
    settingsLayout->setSpacing(6);
    settingsLayout->setContentsMargins(6, 6, 6, 6);
    settingsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    settingsLayout->addWidget(m_checkForUpdatesAutomaticallyCheckBox, 0, 0);

    m_settingsGroup->setTitle(tr("Settings"));
    m_checkForUpdatesAutomaticallyCheckBox->setText(tr("Check for updates automatically"));

    m_checkForUpdatesAutomaticallyCheckBox->setCursor(Qt::PointingHandCursor);

    /****/

    fill();

    connect(m_checkUpdatesButton, &QPushButton::clicked,
            this, [=] { UpdateManager::scheduleUpdateCheck(); });
    connect(m_updateButton, &QPushButton::clicked,
            this, [=] { UpdateManager::update(); });
    connect(ServerManager::instance(), &ServerManager::stateChanged,
            this, &UpdateSettingsWidget::updateCheckButton);
    connect(UpdateManager::instance(), &UpdateManager::updateCheckStarted,
            this, &UpdateSettingsWidget::updateCheckButton);
    connect(UpdateManager::instance(), &UpdateManager::updateCheckFinished,
            this, &UpdateSettingsWidget::updateCheckButton);
    connect(UpdateManager::instance(), &UpdateManager::updateCheckFinished,
            this, [this] (bool succeed) {
        if (succeed) {
            UpdateSettings* settings = SystemSettings::updateSettings();
            settings->lastUpdateCheckDate = QDateTime::currentDateTime();
            settings->write();
            m_upToDateLabel->setText(tr("Objectwheel is up to date"));
            m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/up-to-date.svg"), QSize(80, 80), this));
            m_lastCheckedDateLabel->setText(settings->lastUpdateCheckDate.toString(Qt::SystemLocaleLongDate));
            const qint64 downloadSize = UpdateManager::downloadSize();
            if (downloadSize > 0) {
                m_updatesAvailableLabel->setText(tr("Updates are available for Objectwheel (%1):")
                                                 .arg(UtilityFunctions::toPrettyBytesString(downloadSize)));
                m_changelogEdit->setHtml(UpdateManager::changelog());
                m_updateStatusStackedLayout->setCurrentWidget(m_updatesAvailableWidget);
                mark();
            }
        }
    });

    activate();
    revert();
}

void UpdateSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    UpdateSettings* settings = SystemSettings::updateSettings();
    /****/
    settings->checkForUpdatesAutomatically = m_checkForUpdatesAutomaticallyCheckBox->isChecked();
    // settings->lastUpdateCheckDate = QDateTime::fromString(m_lastCheckedDateLabel->text(), Qt::SystemLocaleLongDate);
    /****/
    settings->write();
}

void UpdateSettingsWidget::revert()
{
    if (!isActivated())
        return;

    activate(false);

    const UpdateSettings* settings = SystemSettings::updateSettings();
    /****/
    m_checkForUpdatesAutomaticallyCheckBox->setChecked(settings->checkForUpdatesAutomatically);

    const QDateTime& lastChecked = SystemSettings::updateSettings()->lastUpdateCheckDate;
    const qint64 days = lastChecked.daysTo(QDateTime::currentDateTime());
    if (!lastChecked.isValid() || days > 2) {
        if (lastChecked.isValid())
            m_upToDateLabel->setText(tr("Updates has not been checked for %1 days").arg(days));
        else
            m_upToDateLabel->setText(tr("Updates has never been checked"));
        m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/update-warning.svg"), QSize(80, 80), this));
    } else {
        m_upToDateLabel->setText(tr("Objectwheel is up to date"));
    }
    if (lastChecked.isValid())
        m_lastCheckedDateLabel->setText(settings->lastUpdateCheckDate.toString(Qt::SystemLocaleLongDate));
    else
        m_lastCheckedDateLabel->setText(tr("Never"));
}

void UpdateSettingsWidget::reset()
{
    SystemSettings::updateSettings()->reset();
    SystemSettings::updateSettings()->write();
    activate();
    revert();
    emit SystemSettings::instance()->designerStateReset();
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

void UpdateSettingsWidget::updateCheckButton()
{
    m_checkUpdatesButton->setEnabled(ServerManager::isConnected() && !UpdateManager::isUpdateCheckRunning());
}

void UpdateSettingsWidget::fill()
{
}