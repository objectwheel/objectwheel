#ifndef BEHAVIORSETTINGSWIDGET_H
#define BEHAVIORSETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QCheckBox;

struct BehaviorSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit BehaviorSettingsWidget(QWidget* parent = nullptr);

    void apply() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    QGroupBox* m_savingGroup;
    QVBoxLayout* m_savingLayout;
    QCheckBox* m_autoSaveAfterRunningBox;
};

#endif // BEHAVIORSETTINGSWIDGET_H
