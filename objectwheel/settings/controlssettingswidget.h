#ifndef CONTROLSSETTINGSWIDGET_H
#define CONTROLSSETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QComboBox;
class QGroupBox;

class ControlsSettingsWidget final : public SettingsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlsSettingsWidget)

public:
    explicit ControlsSettingsWidget(QWidget* parent = nullptr);
    void apply() override;
    void revert() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    void fill();

private:
    QGroupBox* m_functionGroup;
    QLabel* m_itemDoubleClickActionLabel;
    QComboBox* m_itemDoubleClickActionBox;
};

#endif // CONTROLSSETTINGSWIDGET_H
