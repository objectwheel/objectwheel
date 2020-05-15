#include <updatesettingswidget.h>
#include <updatesettings.h>
#include <generalsettings.h>
#include <utilityfunctions.h>
#include <qtcolorbutton.h>
#include <paintutils.h>
#include <servermanager.h>
#include <updatemanager.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QFontDatabase>

UpdateSettingsWidget::UpdateSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_currentStatusGroup(new QGroupBox(contentWidget()))
  , m_logoLabel(new QLabel(m_currentStatusGroup))
  , m_brandLabel(new QLabel(m_currentStatusGroup))
  , m_versionLabel(new QLabel(m_currentStatusGroup))
  , m_revisionLabel(new QLabel(m_currentStatusGroup))
  , m_buildDateLabel(new QLabel(m_currentStatusGroup))
  /****/
  , m_updateGroup(new QGroupBox(contentWidget()))
{
    contentLayout()->addWidget(m_currentStatusGroup);
    contentLayout()->addWidget(m_updateGroup);
    contentLayout()->addStretch();

    /****/

    auto currentStatusLayout = new QGridLayout(m_currentStatusGroup);
    currentStatusLayout->setSpacing(6);
    currentStatusLayout->setContentsMargins(6, 6, 6, 6);
    currentStatusLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    currentStatusLayout->addWidget(m_logoLabel, 0, 0, 6 , 1, Qt::AlignLeft | Qt::AlignVCenter);
    currentStatusLayout->addWidget(m_brandLabel, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentStatusLayout->addWidget(m_versionLabel, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentStatusLayout->addWidget(m_revisionLabel, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentStatusLayout->addWidget(m_buildDateLabel, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);
    currentStatusLayout->setRowStretch(0, 1);
    currentStatusLayout->setRowStretch(5, 1);
    currentStatusLayout->setColumnStretch(3, 1);
    currentStatusLayout->setColumnMinimumWidth(1, 6);

    m_currentStatusGroup->setTitle(tr("Current Status"));
    m_brandLabel->setText(QLatin1String("<span style=\"font-size:14px;\">Objectwheel (Beta)</span>"));
    m_versionLabel->setText(tr("Version: ") + QStringLiteral(APP_VER));
    m_revisionLabel->setText(tr("Revision: ") + QStringLiteral(APP_GITHASH));
    m_buildDateLabel->setText(tr("Build Date: ") + QStringLiteral(APP_GITDATE));

    m_logoLabel->setFixedSize(QSize(112, 112));
    m_logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/icon.png"), QSize(112, 112), this));

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