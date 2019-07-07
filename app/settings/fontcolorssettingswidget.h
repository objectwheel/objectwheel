#ifndef FONTCOLORSSETTINGSWIDGET_H
#define FONTCOLORSSETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QPushButton;

namespace TextEditor { namespace Internal {
class SchemeListModel;
class ColorSchemeEdit;
} }

class FontColorsSettingsWidget final : public SettingsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FontColorsSettingsWidget)

public:
    explicit FontColorsSettingsWidget(QWidget* parent = nullptr);
    void apply() override;
    void revert() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private slots:
    void onFontOptionsChange();
    void onColorOptionsChange(int index);
    void onFontResetButtonClick();
    void onColorSchemeCopyButtonClick();
    void onColorSchemeDeleteButtonClick();

private:
    void fill();
    void maybeSaveColorScheme(const QString& fileName);
    void setCurrentColorScheme(const QString& fileName);

private:
    bool m_disableSaving;
    /****/
    QGroupBox* m_fontGroup;
    QLabel* m_fontFamilyLabel;
    QLabel* m_fontSizeLabel;
    QComboBox* m_fontFamilyBox;
    QComboBox* m_fontSizeBox;
    QCheckBox* m_fontAntialiasingBox;
    QCheckBox* m_fontThickBox;
    QPushButton* m_fontResetButton;
    /****/
    TextEditor::Internal::SchemeListModel* m_schemeListModel;
    QGroupBox* m_colorSchemeGroup;
    QComboBox* m_colorSchemeBox;
    QPushButton* m_colorSchemeCopyButton;
    QPushButton* m_colorSchemeDeleteButton;
    TextEditor::Internal::ColorSchemeEdit* m_colorSchemeEdit;
};

#endif // FONTCOLORSSETTINGSWIDGET_H
