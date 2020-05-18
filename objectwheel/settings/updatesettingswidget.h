#ifndef UPDATESETTINGSWIDGET_H
#define UPDATESETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QCheckBox;
class QPushButton;
class QStackedLayout;
class QPlainTextEdit;

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

private:
    void fill();

private:
    QGroupBox* m_updateSettingsGroup;
    QCheckBox* m_checkForUpdatesAutomaticallyCheckBox;
    /****/
    QGroupBox* m_installedVersionGroup;
    QLabel* m_logoLabel;
    QLabel* m_brandIconLabel;
    QLabel* m_versionLabel;
    QLabel* m_revisionLabel;
    QLabel* m_buildDateLabel;
    /****/
    QGroupBox* m_updateGroup;
    QStackedLayout* m_updateStatusStackedLayout;
    /*__*/
    QWidget* m_upToDateWidget;
    QLabel* m_upToDateLabel;
    QLabel* m_lastCheckedLabel;
    QLabel* m_lastCheckedDateLabel;
    QLabel* m_upToDateIcon;
    QPushButton* m_checkUpdatesButton;
    /*__*/
    QWidget* m_updatesAvailableWidget;
    QLabel* m_updatesAvailableLabel;
    QLabel* m_updatesAvailableIcon;
    QPlainTextEdit* m_changelogEdit;
    QPushButton* m_updateButton;
};

#endif // UPDATESETTINGSWIDGET_H
