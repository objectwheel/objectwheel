#ifndef TOOLBOXSETTINGSWIDGET_H
#define TOOLBOXSETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QCheckBox;
class QComboBox;
class QGroupBox;

class ToolboxSettingsWidget final : public SettingsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolboxSettingsWidget)

public:
    explicit ToolboxSettingsWidget(QWidget* parent = nullptr);
    void apply() override;
    void revert() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    void fill();

private:
    QGroupBox* m_appearanceGroup;
    QLabel* m_alternatingRowColorsLabel;
    QLabel* m_textElideModeLabel;
    QLabel* m_iconSizeLabel;
    QCheckBox* m_alternatingRowColorsCheckBox;
    QComboBox* m_textElideModeBox;
    QComboBox* m_iconSizeBox;
};

#endif // TOOLBOXSETTINGSWIDGET_H
