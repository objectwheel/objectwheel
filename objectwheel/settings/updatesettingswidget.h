#ifndef UPDATESETTINGSWIDGET_H
#define UPDATESETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QPushButton;
namespace Utils { class QtColorButton; }

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
    bool m_themeMessageShowed;
    bool m_hdpiMessageShowed;
    bool m_fontMessageShowed;
    bool m_languageMessageShowed;
    /****/
    QGroupBox* m_updateGroup;
    QLabel* m_themeLabel;
    QLabel* m_languageLabel;
    QLabel* m_hdpiLabel;
    QLabel* m_highlightColorLabel;
    QComboBox* m_themeBox;
    QComboBox* m_languageBox;
    QCheckBox* m_hdpiCheckBox;
    Utils::QtColorButton* m_highlightColorButton;
    QPushButton* m_highlightColorResetButton;
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
    QGroupBox* m_behavioralGroup;
    QCheckBox* m_outputPanePopsCheckBox;
    QCheckBox* m_outputPaneMinimizedStartupCheckBox;
    QCheckBox* m_preserveDesignerStateCheckBox;
    QPushButton* m_designerStateResetButton;
};

#endif // UPDATESETTINGSWIDGET_H
