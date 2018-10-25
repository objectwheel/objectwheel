#ifndef FONTCOLORSSETTINGSWIDGET_H
#define FONTCOLORSSETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QVBoxLayout;
class QPushButton;

struct FontColorsSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit FontColorsSettingsWidget(QWidget* parent = nullptr);

    void apply() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    QGroupBox* m_fontGroup;
    QVBoxLayout* m_fontLayout;
    QLabel* m_fontFamilyLabel;
    QLabel* m_fontSizeLabel;
    QComboBox* m_fontFamilyBox;
    QComboBox* m_fontSizeBox;
    QCheckBox* m_fontAntialiasingBox;
    QCheckBox* m_fontThickBox;
    QPushButton* m_fontResetButton;
};

#endif // FONTCOLORSSETTINGSWIDGET_H
