#ifndef UPDATESETTINGSWIDGET_H
#define UPDATESETTINGSWIDGET_H

#include <settingswidget.h>
#include <QAbstractSocket>

class QLabel;
class QGroupBox;
class QCheckBox;
class QPushButton;
class QStackedLayout;
class QTextEdit;
class WaitingSpinnerWidget;

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
    void updateCheckButton();

private:
    void fill();

private:
    QGroupBox* m_updateGroup;
    QStackedLayout* m_updateStatusStackedLayout;
    /*__*/
    QWidget* m_upToDateWidget;
    QLabel* m_upToDateLabel;
    QLabel* m_lastCheckedLabel;
    QLabel* m_lastCheckedDateLabel;
    QLabel* m_upToDateIcon;
    QPushButton* m_checkUpdatesButton;
    WaitingSpinnerWidget* m_updateCheckSpinner;
    /*__*/
    QWidget* m_updatesAvailableWidget;
    QLabel* m_updatesAvailableLabel;
    QLabel* m_updatesAvailableIcon;
    QTextEdit* m_changelogEdit;
    QPushButton* m_updateButton;
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
};

#endif // UPDATESETTINGSWIDGET_H
