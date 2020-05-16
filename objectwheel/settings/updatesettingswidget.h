#ifndef UPDATESETTINGSWIDGET_H
#define UPDATESETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QCheckBox;
class QPushButton;
class QStackedLayout;

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
    QGroupBox* m_currentVersionGroup;
    QLabel* m_logoLabel;
    QLabel* m_brandLabel;
    QLabel* m_versionLabel;
    QLabel* m_revisionLabel;
    QLabel* m_buildDateLabel;
    /****/
    QGroupBox* m_updateGroup;
    QCheckBox* m_checkForUpdatesAutomaticallyCheckBox;
    QPushButton* m_checkUpdatesButton;
    QStackedLayout* m_updateStatusStackedLayout;
};

#endif // UPDATESETTINGSWIDGET_H
