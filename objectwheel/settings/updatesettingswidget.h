#ifndef UPDATESETTINGSWIDGET_H
#define UPDATESETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QCheckBox;
class QPushButton;
class StackedLayout;
class QTextEdit;
class BusyIndicatorWidget;
class QProgressBar;

class UpdateSettingsWidget final : public SettingsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateSettingsWidget)

public:
    explicit UpdateSettingsWidget(QWidget* parent = nullptr);
    void apply() override;
    void revert() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private slots:
    void onUpdateCheckStatusChange();

private:
    QGroupBox* m_updateGroup;
    StackedLayout* m_updateStatusStackedLayout;
    /*__*/
    QWidget* m_upToDateWidget;
    QLabel* m_upToDateLabel;
    QLabel* m_lastCheckedLabel;
    QLabel* m_lastCheckedDateLabel;
    QLabel* m_upToDateIcon;
    QPushButton* m_checkUpdatesButton;
    BusyIndicatorWidget* m_busyIndicator;
    /*__*/
    QWidget* m_updatesAvailableWidget;
    QLabel* m_updatesAvailableLabel;
    QLabel* m_updatesAvailableIcon;
    QTextEdit* m_changelogEdit;
    QPushButton* m_downloadButton;
    /*__*/
    QWidget* m_downloadWidget;
    QLabel* m_downloadingIcon;
    QLabel* m_downloadingLabel;
    QPushButton* m_abortAndInstallButton;
    QProgressBar* m_downloadProgressBar;
    QLabel* m_downloadSizeLabel;
    QLabel* m_downloadSpeedLabel;
    /****/
    QGroupBox* m_statusGroup;
    QLabel* m_logoLabel;
    QLabel* m_brandIconLabel;
    QLabel* m_versionLabel;
    QLabel* m_revisionLabel;
    QLabel* m_buildDateLabel;
    /****/
    QGroupBox* m_settingsGroup;
    QCheckBox* m_checkForUpdatesAutomaticallyCheckBox;
    QPushButton* m_showCacheFolderButton;
    QPushButton* m_showCacheSizeButton;
    QPushButton* m_cleanCacheButton;
};

#endif // UPDATESETTINGSWIDGET_H
