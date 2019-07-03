#ifndef BEHAVIORSETTINGSWIDGET_H
#define BEHAVIORSETTINGSWIDGET_H

#include <settingswidget.h>

class QGroupBox;
class QCheckBox;

class BehaviorSettingsWidget final : public SettingsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BehaviorSettingsWidget)

public:
    explicit BehaviorSettingsWidget(QWidget* parent = nullptr);
    void apply() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    QGroupBox* m_savingGroup;
    QCheckBox* m_autoSaveBeforeRunningBox;
};

#endif // BEHAVIORSETTINGSWIDGET_H
