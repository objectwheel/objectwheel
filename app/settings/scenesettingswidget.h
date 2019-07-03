#ifndef SCENESETTINGSWIDGET_H
#define SCENESETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QGridLayout;
class QVBoxLayout;
class QComboBox;

struct SceneSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit SceneSettingsWidget(QWidget* parent = nullptr);

    void apply() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    QGroupBox* m_designGroup;
    QGridLayout* m_designLayout;
    QLabel* m_showGuideLinesLabel;
    QLabel* m_sceneBackgroundColorModeLabel;
    QLabel* m_sceneZoomLevelLabel;
    QCheckBox* m_showGuideLinesCheckBox;
    QComboBox* m_sceneBackgroundColorModeBox;
    QComboBox* m_sceneZoomLevelBox;
    /****/
    QGroupBox* m_gridViewGroup;
    QGridLayout* m_gridViewLayout;
    QLabel* m_showGridViewDotsLabel;
    QLabel* m_snappingEnabledLabel;
    QLabel* m_gridSizeLabel;
    QCheckBox* m_showGridViewDotsCheckBox;
    QCheckBox* m_snappingEnabledCheckBox;
    QSpinBox* m_gridSizeSpinBox;
    QPushButton* m_resetGridViewButton;
    /****/
    QGroupBox* m_controlsGroup;
    QGridLayout* m_controlsLayout;
    QLabel* m_showMouseoverOutlineLabel;
    QLabel* m_controlOutlineModeLabel;
    QCheckBox* m_showMouseoverOutlineCheckBox;
    QComboBox* m_controlOutlineModeBox;
};

#endif // SCENESETTINGSWIDGET_H
