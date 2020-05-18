#include <updatesettingswidget.h>
#include <updatesettings.h>
#include <generalsettings.h>
#include <paintutils.h>
#include <servermanager.h>
#include <updatemanager.h>

#include <QCoreApplication>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>
#include <QStackedLayout>
#include <QPlainTextEdit>

Q_DECLARE_METATYPE(QMargins)

const char layoutMarginsProperty[] = "_q_ApplicationStyle_layoutMarginsProperty";

UpdateSettingsWidget::UpdateSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_settingsGroup(new QGroupBox(contentWidget()))
  , m_checkForUpdatesAutomaticallyCheckBox(new QCheckBox(m_settingsGroup))
  /****/
  , m_installedVersionGroup(new QGroupBox(contentWidget()))
  , m_logoLabel(new QLabel(m_installedVersionGroup))
  , m_brandIconLabel(new QLabel(m_installedVersionGroup))
  , m_versionLabel(new QLabel(m_installedVersionGroup))
  , m_revisionLabel(new QLabel(m_installedVersionGroup))
  , m_buildDateLabel(new QLabel(m_installedVersionGroup))
  /****/
  , m_updateGroup(new QGroupBox(contentWidget()))
  , m_updateStatusStackedLayout(new QStackedLayout(m_updateGroup))
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
  , m_changelogEdit(new QPlainTextEdit(m_updatesAvailableWidget))
  , m_updateButton(new QPushButton(m_updatesAvailableWidget))
{
    contentLayout()->addWidget(m_settingsGroup);
    contentLayout()->addWidget(m_installedVersionGroup);
    contentLayout()->addWidget(m_updateGroup);
    contentLayout()->addStretch();

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

    auto installedVersionLayout = new QGridLayout(m_installedVersionGroup);
    installedVersionLayout->setSpacing(6);
    installedVersionLayout->setContentsMargins(6, 6, 6, 6);
    installedVersionLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    installedVersionLayout->addWidget(m_logoLabel, 0, 0, 4, 1, Qt::AlignVCenter);
    installedVersionLayout->addWidget(m_brandIconLabel, 0, 1);
    installedVersionLayout->addWidget(m_versionLabel, 1, 1);
    installedVersionLayout->addWidget(m_revisionLabel, 2, 1);
    installedVersionLayout->addWidget(m_buildDateLabel, 3, 1);

    m_installedVersionGroup->setTitle(tr("Installed Version"));
    m_brandIconLabel->setText(QLatin1String("Objectwheel (Beta)"));
    m_versionLabel->setText(tr("Version: ") + QStringLiteral(APP_VER));
    m_revisionLabel->setText(tr("Revision: ") + QStringLiteral(APP_GITHASH));
    m_buildDateLabel->setText(tr("Build Date: ") + QStringLiteral(APP_GITDATE));

    m_logoLabel->setFixedSize(QSize(80, 80));
    m_logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/icon.png"), QSize(80, 80), this));

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
    m_upToDateLabel->setText(QLatin1String("<span style=\"font-weight:500;\">%1</span>")
                             .arg(tr("Objectwheel is up to date")));
    m_lastCheckedLabel->setText(tr("Last checked:"));

    m_upToDateIcon->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, -4, 0, 0)));
    m_upToDateIcon->setFixedSize(QSize(80, 80));
    m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/up-to-date.svg"), QSize(80, 80), this));
    QCoreApplication::postEvent(m_upToDateIcon, new QEvent(QEvent::StyleChange)); // Apply margin change

    m_checkUpdatesButton->setCursor(Qt::PointingHandCursor);
    m_checkUpdatesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    // FIXME: remove this
    m_lastCheckedDateLabel->setText(APP_GITDATE);

    /*__*/

    auto updatesAvailableLayout = new QGridLayout(m_updatesAvailableWidget);
    updatesAvailableLayout->setSpacing(6);
    updatesAvailableLayout->setContentsMargins(6, 6, 6, 6);
    updatesAvailableLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    updatesAvailableLayout->addWidget(m_updatesAvailableLabel, 0, 0, 1, 3);
    updatesAvailableLayout->addWidget(m_updatesAvailableIcon, 1, 0);
    updatesAvailableLayout->addWidget(m_changelogEdit, 1, 2, 3, 1);
    updatesAvailableLayout->addWidget(m_updateButton, 3, 0, 1, 1);

    m_updateButton->setText(tr("Update"));
    m_updatesAvailableLabel->setText(QLatin1String("<span style=\"font-weight:500;\">%1</span>")
                                     .arg(tr("Updates are available for Objectwheel")));

    m_updatesAvailableIcon->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, -5, 0, 0)));
    m_updatesAvailableIcon->setFixedSize(QSize(80, 80));
    m_updatesAvailableIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/updates-available.svg"), QSize(80, 80), this));
    QCoreApplication::postEvent(m_updatesAvailableIcon, new QEvent(QEvent::StyleChange)); // Apply margin change

    /****/


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