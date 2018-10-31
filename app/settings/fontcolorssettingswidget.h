#ifndef FONTCOLORSSETTINGSWIDGET_H
#define FONTCOLORSSETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QGridLayout;
class SchemeListModel;

namespace TextEditor { namespace Internal { class ColorSchemeEdit; } }

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

private slots:
    void onFontOptionsChange();
    void onColorOptionsChange(int index);

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
    /****/
    SchemeListModel* m_schemeListModel;
    QGroupBox* m_colorSchemeGroup;
    QGridLayout* m_colorSchemeLayout;
    QComboBox* m_colorSchemeBox;
    QPushButton* m_colorSchemeCopyButton;
    QPushButton* m_colorSchemeDeleteButton;
    TextEditor::Internal::ColorSchemeEdit* m_colorSchemeEdit;
};

#endif // FONTCOLORSSETTINGSWIDGET_H
