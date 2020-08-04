#include <updatesettingswidget.h>
#include <updatesettings.h>
#include <systemsettings.h>
#include <paintutils.h>
#include <updatemanager.h>
#include <stackedlayout.h>
#include <busyindicatorwidget.h>
#include <appconstants.h>
#include <utilityfunctions.h>
#include <applicationcore.h>
#include <fileutils.h>

#include <QFileInfo>
#include <QProgressBar>
#include <QCoreApplication>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
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
  , m_busyIndicator(new BusyIndicatorWidget(m_upToDateWidget, false, false))
  /*__*/
  , m_updatesAvailableWidget(new QWidget(m_updateGroup))
  , m_updatesAvailableLabel(new QLabel(m_updatesAvailableWidget))
  , m_updatesAvailableIcon(new QLabel(m_updatesAvailableWidget))
  , m_changelogEdit(new QTextEdit(m_updatesAvailableWidget))
  , m_downloadButton(new QPushButton(m_updatesAvailableWidget))
  /*__*/
  , m_downloadWidget(new QWidget(m_updateGroup))
  , m_downloadingIcon(new QLabel(m_downloadWidget))
  , m_downloadingLabel(new QLabel(m_downloadWidget))
  , m_abortAndInstallButton(new QPushButton(m_downloadWidget))
  , m_downloadProgressBar(new QProgressBar(m_downloadWidget))
  , m_downloadSizeLabel(new QLabel(m_downloadWidget))
  , m_downloadSpeedLabel(new QLabel(m_downloadWidget))
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
  , m_showCacheFolderButton(new QPushButton(m_settingsGroup))
  , m_showCacheSizeButton(new QPushButton(m_settingsGroup))
  , m_cleanCacheButton(new QPushButton(m_settingsGroup))
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
    m_updateStatusStackedLayout->addWidget(m_downloadWidget);

    m_updateGroup->setTitle(tr("Update"));

    /*__*/

    auto upToDateLayout = new QGridLayout(m_upToDateWidget);
    upToDateLayout->setSpacing(6);
    upToDateLayout->setContentsMargins(6, 6, 6, 6);
    upToDateLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    upToDateLayout->addWidget(m_upToDateIcon, 0, 0, 4, 1, Qt::AlignTop);
    upToDateLayout->addWidget(m_upToDateLabel, 0, 1, 1, 4);
    upToDateLayout->addWidget(m_lastCheckedLabel, 1, 1, 1, 3);
    upToDateLayout->addWidget(m_lastCheckedDateLabel, 1, 4);
    upToDateLayout->addWidget(m_checkUpdatesButton, 2, 1);
    upToDateLayout->addWidget(m_busyIndicator, 2, 2);
    upToDateLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 2, 3);
    upToDateLayout->setRowStretch(3, 1);
    upToDateLayout->setColumnStretch(4, 1);

    m_lastCheckedLabel->setSizePolicy(QSizePolicy::Maximum, m_lastCheckedLabel->sizePolicy().verticalPolicy());
    m_checkUpdatesButton->setSizePolicy(QSizePolicy::Maximum, m_checkUpdatesButton->sizePolicy().verticalPolicy());
    m_busyIndicator->setSizePolicy(QSizePolicy::Maximum, m_busyIndicator->sizePolicy().verticalPolicy());

    m_checkUpdatesButton->setText(tr("Check Now"));
    m_lastCheckedLabel->setText(tr("Last successful check:"));

    m_checkUpdatesButton->setToolTip(tr("Make a fresh update check"));
    m_lastCheckedDateLabel->setToolTip(tr("Shows last successful update check date"));

    m_upToDateIcon->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, -4, 0, 0)));
    m_upToDateIcon->setFixedSize(QSize(80, 80));
    QCoreApplication::postEvent(m_upToDateIcon, new QEvent(QEvent::StyleChange)); // Apply margin change

    m_checkUpdatesButton->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(-1, 0, 0, 0)));
    m_checkUpdatesButton->setCursor(Qt::PointingHandCursor);
    QCoreApplication::postEvent(m_checkUpdatesButton, new QEvent(QEvent::StyleChange)); // Apply margin change

    m_busyIndicator->setLineWidth(2);
    m_busyIndicator->setRoundness(50);
    m_busyIndicator->setLineLength(5);
    m_busyIndicator->setInnerRadius(4);
    m_busyIndicator->setNumberOfLines(12);
    m_busyIndicator->setMinimumTrailOpacity(5);
    m_busyIndicator->setRevolutionsPerSecond(2);
    m_busyIndicator->setTrailFadePercentage(100);
    m_busyIndicator->setStyleSheet("background: transparent;");
    m_busyIndicator->setColor(palette().text().color());

    /*__*/

    auto updatesAvailableLayout = new QGridLayout(m_updatesAvailableWidget);
    updatesAvailableLayout->setSpacing(6);
    updatesAvailableLayout->setContentsMargins(6, 6, 6, 6);
    updatesAvailableLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    updatesAvailableLayout->addWidget(m_downloadButton, 0, 0);
    updatesAvailableLayout->addWidget(m_updatesAvailableLabel, 0, 1);
    updatesAvailableLayout->addWidget(m_updatesAvailableIcon, 1, 0, Qt::AlignTop);
    updatesAvailableLayout->addWidget(m_changelogEdit, 1, 1);

    m_downloadButton->setText(tr("Download"));

    m_updatesAvailableIcon->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, -7, 0, 0)));
    m_updatesAvailableIcon->setFixedSize(QSize(80, 80));
    m_updatesAvailableIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/updates-available.svg"), QSize(80, 80), this));
    QCoreApplication::postEvent(m_updatesAvailableIcon, new QEvent(QEvent::StyleChange)); // Apply margin change

    m_changelogEdit->setReadOnly(true);
    UtilityFunctions::adjustFontPixelSize(m_changelogEdit, -1);
    m_changelogEdit->setMinimumHeight(170);

    m_downloadButton->setCursor(Qt::PointingHandCursor);
    m_downloadButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    /*__*/

    auto downloadLayout = new QGridLayout(m_downloadWidget);
    downloadLayout->setSpacing(6);
    downloadLayout->setContentsMargins(6, 6, 6, 6);
    downloadLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    downloadLayout->addWidget(m_downloadingIcon, 0, 0, 4, 1, Qt::AlignTop);
    downloadLayout->addWidget(m_downloadingLabel, 0, 1);
    downloadLayout->addWidget(m_abortAndInstallButton, 0, 2);
    downloadLayout->addWidget(m_downloadProgressBar, 1, 1, 1, 2);
    downloadLayout->addWidget(m_downloadSizeLabel, 2, 1);
    downloadLayout->addWidget(m_downloadSpeedLabel, 2, 2);
    downloadLayout->setRowStretch(3, 1);

    m_abortAndInstallButton->setText(tr("Abort"));
    m_downloadingLabel->setText(tr("Downloading..."));
    m_downloadSizeLabel->setText("0000.00 MB / 0000.00 MB");
    m_downloadSpeedLabel->setText("0000.00 MB/s ↓");
    m_downloadProgressBar->setPalette(QColor("#65b84b")); // Button

    // NOTE: QProgressBar::minimumSizeHint() returns fontMetrics().height() + 2 for the height
    m_downloadProgressBar->setFixedHeight(m_downloadProgressBar->sizeHint().height());
    m_downloadSizeLabel->setFixedHeight(m_abortAndInstallButton->sizeHint().height());
    m_downloadSpeedLabel->setFixedHeight(m_abortAndInstallButton->sizeHint().height());
    m_downloadSpeedLabel->setFixedWidth(m_downloadSpeedLabel->fontMetrics().
                                        horizontalAdvance(m_downloadSpeedLabel->text()) + 2);

    m_downloadSpeedLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_downloadSizeLabel->setAlignment(Qt::AlignVCenter);
    m_downloadingLabel->setAlignment(Qt::AlignVCenter);

    m_downloadingIcon->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, -4, 0, 0)));
    m_downloadingIcon->setFixedSize(QSize(80, 80));
    m_downloadingIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/downloading.svg"), QSize(80, 80), this));
    QCoreApplication::postEvent(m_downloadingIcon, new QEvent(QEvent::StyleChange)); // Apply margin change

    m_abortAndInstallButton->setProperty(layoutMarginsProperty, QVariant::fromValue(QMargins(0, 0, -1, 0)));
    m_abortAndInstallButton->setCursor(Qt::PointingHandCursor);
    QCoreApplication::postEvent(m_abortAndInstallButton, new QEvent(QEvent::StyleChange)); // Apply margin change

    /****/

    auto infoButton = new QPushButton(m_statusGroup);
    infoButton->setFixedSize(18, 18);
    infoButton->setCursor(Qt::PointingHandCursor);
    infoButton->setToolTip(tr("A quick reminder about updates"));
    infoButton->setIcon(QIcon(":/images/output/info.svg"));
    infoButton->setFlat(true);

    auto statusLayout = new QGridLayout(m_statusGroup);
    statusLayout->setSpacing(6);
    statusLayout->setContentsMargins(6, 6, 6, 6);
    statusLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    statusLayout->addWidget(m_logoLabel, 0, 0, 4, 1, Qt::AlignVCenter);
    statusLayout->addWidget(m_brandIconLabel, 0, 1);
    statusLayout->addWidget(infoButton, 0, 2);
    statusLayout->addWidget(m_versionLabel, 1, 1);
    statusLayout->addWidget(m_revisionLabel, 2, 1);
    statusLayout->addWidget(m_buildDateLabel, 3, 1);
    statusLayout->setColumnStretch(1, 1);

    m_statusGroup->setTitle(tr("Status"));
    m_brandIconLabel->setText(AppConstants::LABEL);
    m_versionLabel->setText(tr("Version: ") + AppConstants::VERSION);
    m_revisionLabel->setText(tr("Revision: ") + AppConstants::REVISION);
    m_buildDateLabel->setText(tr("Build date: ") + QDateTime::fromString(AppConstants::BUILD_DATE, Qt::ISODate).toString(Qt::SystemLocaleLongDate));

    m_logoLabel->setFixedSize(QSize(80, 80));
    m_logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/icon.png"), QSize(80, 80), this));

    /****/

    auto settingsLayout = new QGridLayout(m_settingsGroup);
    settingsLayout->setSpacing(6);
    settingsLayout->setContentsMargins(6, 6, 6, 6);
    settingsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    settingsLayout->addWidget(m_checkForUpdatesAutomaticallyCheckBox, 0, 0, 1, 3);
    settingsLayout->addWidget(m_showCacheFolderButton, 1, 0);
    settingsLayout->addWidget(m_showCacheSizeButton, 1, 1);
    settingsLayout->addWidget(m_cleanCacheButton, 1, 2);
    settingsLayout->setColumnStretch(3, 1);

    m_settingsGroup->setTitle(tr("Settings"));
    m_checkForUpdatesAutomaticallyCheckBox->setText(tr("Check for updates automatically"));
    m_checkForUpdatesAutomaticallyCheckBox->setToolTip(tr("It does not download or install updates automatically in any case"));
    m_checkForUpdatesAutomaticallyCheckBox->setCursor(Qt::PointingHandCursor);

    m_showCacheFolderButton->setText(tr("Show Cache Folder"));
    m_showCacheFolderButton->setToolTip(tr("Shows update cache folder"));
    m_showCacheFolderButton->setCursor(Qt::PointingHandCursor);

    m_showCacheSizeButton->setText(tr("Calculate Cache Size"));
    m_showCacheSizeButton->setToolTip(tr("Calculates update cache size (this may freeze the app)."));
    m_showCacheSizeButton->setCursor(Qt::PointingHandCursor);

    m_cleanCacheButton->setText(tr("Clean Update Cache"));
    m_cleanCacheButton->setToolTip(tr("Deletes existing download files and cleans up the update cache"));
    m_cleanCacheButton->setCursor(Qt::PointingHandCursor);

    /****/

    connect(infoButton, &QPushButton::clicked, this, [=] {
        UtilityFunctions::showMessage(
                    this,
                    tr("A quick reminder about updates"),
                    tr("Objectwheel needs regular updates to function properly. "
                       "Opening up new projects or using cloud services with an "
                       "outdated version of Objectwheel may result in failure."
                       "\n\n"
                       "Meanwhile Objectwheel does not download or install any "
                       "updates automatically in any way."),
                    QMessageBox::Information);
    });
    connect(m_checkForUpdatesAutomaticallyCheckBox, &QCheckBox::clicked, this, [=] {
        if (!m_checkForUpdatesAutomaticallyCheckBox->isChecked()) {
            QMessageBox::StandardButton ret = UtilityFunctions::showMessage(
                        this,
                        tr("Are you sure?"),
                        tr("Objectwheel needs regular updates to function properly. "
                           "Opening up new projects or using cloud services with an "
                           "outdated version of Objectwheel may result in failure."),
                        QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::No)
                m_checkForUpdatesAutomaticallyCheckBox->setChecked(true);
        }
    });
    connect(m_showCacheFolderButton, &QPushButton::clicked, this, [=] {
        if (QFileInfo::exists(ApplicationCore::updatesPath())) {
            Utils::FileUtils::showInFolder(this, ApplicationCore::updatesPath());
        } else {
            UtilityFunctions::showMessage(this, tr("No cache found"),
                                          tr("There is nothing to worry about!"),
                                          QMessageBox::Information);
        }
    });
    connect(m_showCacheSizeButton, &QPushButton::clicked, this, [=] {
        m_showCacheSizeButton->setEnabled(false);
        UtilityFunctions::showMessage(this, tr("Calculation is done"), tr("Update cache size: ") +
                                      UtilityFunctions::toPrettyBytesString(UpdateManager::cacheSize()),
                                      QMessageBox::Information);
        m_showCacheSizeButton->setEnabled(true);
    });
    connect(m_cleanCacheButton, &QPushButton::clicked, this, [=] {
        if (UpdateManager::isDownloadRunning()) {
            UtilityFunctions::showMessage(this, tr("Operation is not permitted"),
                                          tr("The update cache cannot be cleaned while an update"
                                             "is already in progress, please abort it first."),
                                          QMessageBox::Warning);
        } else {
            QMessageBox::StandardButton ret =
                    UtilityFunctions::showMessage(this, tr("Are you sure?"),
                                                  tr("This will clean up the update cache, interrupted "
                                                     "updates will start downloading over again."),
                                                  QMessageBox::Question, QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::No);
            if (ret == QMessageBox::No)
                return;
            UpdateManager::cleanCache();
            if (m_downloadProgressBar->invertedAppearance()) {
                revert();
                m_updateStatusStackedLayout->setCurrentWidget(m_upToDateWidget);
            }
            m_showCacheSizeButton->click();
        }
    });
    connect(m_checkUpdatesButton, &QPushButton::clicked,
            this, [=] { UpdateManager::startUpdateCheck(); });
    connect(m_downloadButton, &QPushButton::clicked, this, [this] {
        if (UpdateManager::fileCount() > 200) {
            QMessageBox::StandardButton ret =
                    UtilityFunctions::showMessage(this, tr("This looks like to be a big update"),
                                                  tr("<p>Fragmented updates with more than 200 files may take "
                                                     "longer to download than what you may expect. We "
                                                     "suggest you to download an offline installer instead from: "
                                                     "<a href='%1'>%1</a></p>"
                                                     "<p>Do you still want to proceed with the update?</p>")
                                                  .arg(AppConstants::DOWNLOAD_URL),
                                                  QMessageBox::Question, QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::No);
            if (ret == QMessageBox::No)
                return;
        }
        UpdateManager::download();
        m_updateStatusStackedLayout->setCurrentWidget(m_downloadWidget);
    });
    connect(UpdateManager::instance(), &UpdateManager::updateCheckStarted,
            this, &UpdateSettingsWidget::onUpdateCheckStatusChange);
    connect(UpdateManager::instance(), &UpdateManager::updateCheckFinished,
            this, &UpdateSettingsWidget::onUpdateCheckStatusChange);
    connect(UpdateManager::instance(), &UpdateManager::updateCheckFinished, this, [this] (bool succeed) {
        if (succeed) {
            int fileCount = UpdateManager::fileCount();
            UpdateSettings* settings = SystemSettings::updateSettings();
            settings->lastSuccessfulUpdateCheckDate = QDateTime::currentDateTime();
            settings->wereUpdatesAvailableLastTime = fileCount > 0;
            settings->write();
            if (fileCount > 0) {
                m_updatesAvailableLabel->setText(tr("Updates are available for %1 (%2 files, %3 in size):")
                                                 .arg(AppConstants::NAME)
                                                 .arg(fileCount)
                                                 .arg(UtilityFunctions::toPrettyBytesString(UpdateManager::downloadSize())));
                m_changelogEdit->setHtml(UpdateManager::changelog());
                m_updateStatusStackedLayout->setCurrentWidget(m_updatesAvailableWidget);
                mark();
            } else {
                m_upToDateLabel->setText(tr("%1 is up to date").arg(AppConstants::NAME));
                m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/up-to-date.svg"), QSize(80, 80), this));
                m_lastCheckedDateLabel->setText(settings->lastSuccessfulUpdateCheckDate.toString(Qt::SystemLocaleLongDate));
                m_updateStatusStackedLayout->setCurrentWidget(m_upToDateWidget);
            }
        } else {
            m_upToDateLabel->setText(tr("Unable to connect to the update server"));
            m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/update-warning.svg"), QSize(80, 80), this));
            m_updateStatusStackedLayout->setCurrentWidget(m_upToDateWidget);
        }
        m_downloadProgressBar->setInvertedAppearance(false);
        m_downloadProgressBar->setValue(0);
        m_downloadingLabel->setText(tr("Downloading..."));
        m_abortAndInstallButton->setText(tr("Abort"));
        m_downloadSizeLabel->setText("0000.00 MB / 0000.00 MB");
        m_downloadSpeedLabel->setText("0000.00 MB/s ↓");
    });
    connect(m_abortAndInstallButton, &QPushButton::clicked, this, [this] {
        if (UpdateManager::isUpdateCheckRunning())
            return;
        if (m_downloadProgressBar->invertedAppearance()) {
            m_abortAndInstallButton->setEnabled(false);
            UpdateSettings* settings = SystemSettings::updateSettings();
            settings->lastSuccessfulUpdateCheckDate = QDateTime::currentDateTime();
            settings->wereUpdatesAvailableLastTime = false;
            settings->write();
            UpdateManager::install();
        } else {
            QMessageBox::StandardButton ret =
                    UtilityFunctions::showMessage(this, tr("Are you sure?"),
                                                  tr("This will abort the download, though files "
                                                     "are cached, so you can continue later."),
                                                  QMessageBox::Question, QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::No);
            if (ret == QMessageBox::No)
                return;
            if (UpdateManager::isDownloadRunning()) {
                m_abortAndInstallButton->setEnabled(false);
                UpdateManager::cancelDownload();
            }
        }
    });
    connect(UpdateManager::instance(), &UpdateManager::downloadProgress, this, [this]
            (qreal bytesPerSec, qint64 bytesReceived, qint64 fileIndex, const QString& fileName) {
        const qint64 totalBytes = UpdateManager::downloadSize();
        const qreal progress = totalBytes > 0 ? 100.0 * bytesReceived / totalBytes : 0;
        const QString& speedStr = UtilityFunctions::toPrettyBytesString(bytesPerSec) + QStringLiteral("/s ↓");
        const QString& szStr = UtilityFunctions::toPrettyBytesString(bytesReceived) + QLatin1String(" / ")
                + UtilityFunctions::toPrettyBytesString(totalBytes)
                + QStringLiteral(" ( % %1 )").arg(QString::number(progress, 'f', 2));
        const QString& dlStr = tr("Downloading (%1 / %2): ").arg(fileIndex).arg(UpdateManager::fileCount());
        m_downloadProgressBar->setValue(progress);
        m_downloadSizeLabel->setText(szStr);
        m_downloadSpeedLabel->setText(speedStr);
        const int w = m_downloadingLabel->width() - m_downloadingLabel->fontMetrics().horizontalAdvance(dlStr) - 1;
        m_downloadingLabel->setText(dlStr + m_downloadingLabel->fontMetrics().elidedText(fileName, Qt::ElideLeft, w));
        UtilityFunctions::updateToolTip(m_downloadingLabel, dlStr + fileName);
    });
    connect(UpdateManager::instance(), &UpdateManager::downloadFinished, this, [this]
            (bool canceled, const QString& errorString) {
        if (canceled) {
            m_updateStatusStackedLayout->setCurrentWidget(m_updatesAvailableWidget);
            m_abortAndInstallButton->setEnabled(true);
            m_downloadProgressBar->setValue(0);
            m_abortAndInstallButton->setText(tr("Abort"));
            m_downloadingLabel->setText(tr("Downloading..."));
            m_downloadSizeLabel->setText("0000.00 MB / 0000.00 MB");
            m_downloadSpeedLabel->setText("0000.00 MB/s ↓");
        } else {
            if (!errorString.isEmpty()) {
                UtilityFunctions::showMessage(this, tr("Something went wrong"), errorString, QMessageBox::Critical);
                m_updateStatusStackedLayout->setCurrentWidget(m_updatesAvailableWidget);
                m_downloadProgressBar->setValue(0);
                m_abortAndInstallButton->setText(tr("Abort"));
                m_downloadingLabel->setText(tr("Downloading..."));
                m_downloadSizeLabel->setText("0000.00 MB / 0000.00 MB");
                m_downloadSpeedLabel->setText("0000.00 MB/s ↓");
            } else {
                m_downloadProgressBar->setInvertedAppearance(true);
                m_abortAndInstallButton->setText(tr("Install"));
                QString text = m_downloadingLabel->text();
                text.replace(tr("Downloading"), tr("Ready to install"));
                m_downloadingLabel->setText(text);
                text = m_downloadingLabel->toolTip();
                text.replace(tr("Downloading"), tr("Ready to install"));
                UtilityFunctions::updateToolTip(m_downloadingLabel, text);
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

    const QDateTime& lastChecked = SystemSettings::updateSettings()->lastSuccessfulUpdateCheckDate;
    const qint64 days = lastChecked.daysTo(QDateTime::currentDateTime());
    if (!lastChecked.isValid()) {
        m_upToDateLabel->setText(tr("Updates has never been checked"));
        m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/update-warning.svg"), QSize(80, 80), this));
        m_lastCheckedDateLabel->setText(tr("Never"));
    } else if (days > 2) {
        m_upToDateLabel->setText(tr("Updates has not been checked for %1 days").arg(days));
        m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/update-warning.svg"), QSize(80, 80), this));
        m_lastCheckedDateLabel->setText(settings->lastSuccessfulUpdateCheckDate.toString(Qt::SystemLocaleLongDate));
    } else if (!settings->checkForUpdatesAutomatically) {
        m_upToDateLabel->setText(tr("Automatic update check disabled"));
        m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/updates-disabled.svg"), QSize(80, 80), this));
        m_lastCheckedDateLabel->setText(settings->lastSuccessfulUpdateCheckDate.toString(Qt::SystemLocaleLongDate));
    } else if (settings->wereUpdatesAvailableLastTime) {
        m_upToDateLabel->setText(tr("Updates are available for %1").arg(AppConstants::NAME));
        m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/updates-disabled.svg"), QSize(80, 80), this));
        m_lastCheckedDateLabel->setText(settings->lastSuccessfulUpdateCheckDate.toString(Qt::SystemLocaleLongDate));
    } else {
        m_upToDateLabel->setText(tr("%1 is up to date").arg(AppConstants::NAME));
        m_upToDateIcon->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/settings/up-to-date.svg"), QSize(80, 80), this));
        m_lastCheckedDateLabel->setText(settings->lastSuccessfulUpdateCheckDate.toString(Qt::SystemLocaleLongDate));
    }
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
    return title().contains(word, Qt::CaseInsensitive)
            || m_updateGroup->title().contains(word, Qt::CaseInsensitive)
            || m_statusGroup->title().contains(word, Qt::CaseInsensitive)
            || m_settingsGroup->title().contains(word, Qt::CaseInsensitive)
            || m_upToDateLabel->text().contains(word, Qt::CaseInsensitive)
            || m_lastCheckedLabel->text().contains(word, Qt::CaseInsensitive)
            || m_lastCheckedDateLabel->text().contains(word, Qt::CaseInsensitive)
            || m_lastCheckedDateLabel->toolTip().contains(word, Qt::CaseInsensitive)
            || m_checkUpdatesButton->text().contains(word, Qt::CaseInsensitive)
            || m_checkUpdatesButton->toolTip().contains(word, Qt::CaseInsensitive)
            || m_updatesAvailableLabel->text().contains(word, Qt::CaseInsensitive)
            || m_downloadButton->text().contains(word, Qt::CaseInsensitive)
            || m_downloadingLabel->text().contains(word, Qt::CaseInsensitive)
            || m_abortAndInstallButton->text().contains(word, Qt::CaseInsensitive)
            || m_downloadSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_brandIconLabel->text().contains(word, Qt::CaseInsensitive)
            || m_versionLabel->text().contains(word, Qt::CaseInsensitive)
            || m_revisionLabel->text().contains(word, Qt::CaseInsensitive)
            || m_buildDateLabel->text().contains(word, Qt::CaseInsensitive)
            || m_checkForUpdatesAutomaticallyCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_checkForUpdatesAutomaticallyCheckBox->toolTip().contains(word, Qt::CaseInsensitive)
            || m_showCacheFolderButton->text().contains(word, Qt::CaseInsensitive)
            || m_showCacheFolderButton->toolTip().contains(word, Qt::CaseInsensitive)
            || m_showCacheSizeButton->text().contains(word, Qt::CaseInsensitive)
            || m_showCacheSizeButton->toolTip().contains(word, Qt::CaseInsensitive)
            || m_cleanCacheButton->text().contains(word, Qt::CaseInsensitive)
            || m_cleanCacheButton->toolTip().contains(word, Qt::CaseInsensitive);
}

void UpdateSettingsWidget::onUpdateCheckStatusChange()
{
    if (UpdateManager::isUpdateCheckRunning())
        m_busyIndicator->start();
    else
        m_busyIndicator->stop();
    m_downloadButton->setEnabled(!UpdateManager::isUpdateCheckRunning());
    m_cleanCacheButton->setEnabled(!UpdateManager::isUpdateCheckRunning());
    m_checkUpdatesButton->setEnabled(!UpdateManager::isUpdateCheckRunning());
}
