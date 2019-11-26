#ifndef SCENESETTINGSWIDGET_H
#define SCENESETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QComboBox;
namespace Utils { class QtColorButton; }

class SceneSettingsWidget final : public SettingsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneSettingsWidget)

public:
    explicit SceneSettingsWidget(QWidget* parent = nullptr);
    void apply() override;
    void revert() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    void fill();

private:
    QGroupBox* m_designGroup;
    QLabel* m_showGuideLinesLabel;
    QLabel* m_showAllAnchorsLabel;
    QLabel* m_dragStartDistanceLabel;
    QLabel* m_sceneBackgroundTextureLabel;
    QLabel* m_sceneZoomLevelLabel;
    QLabel* m_anchorColorLabel;
    QCheckBox* m_showGuideLinesCheckBox;
    QCheckBox* m_showAllAnchorsCheckBox;
    QSpinBox* m_dragStartDistanceSpinBox;
    QComboBox* m_sceneBackgroundTextureBox;
    QComboBox* m_sceneZoomLevelBox;
    Utils::QtColorButton* m_anchorColorButton;
    QPushButton* m_anchorColorResetButton;
    /****/
    QGroupBox* m_gridViewGroup;
    QLabel* m_showGridViewDotsLabel;
    QLabel* m_snappingEnabledLabel;
    QLabel* m_gridSizeLabel;
    QCheckBox* m_showGridViewDotsCheckBox;
    QCheckBox* m_snappingEnabledCheckBox;
    QSpinBox* m_gridSizeSpinBox;
    QPushButton* m_resetGridViewButton;
    /****/
    QGroupBox* m_controlsGroup;
    QLabel* m_showMouseoverOutlineLabel;
    QLabel* m_showClippedControlsLabel;
    QLabel* m_blankControlDecorationLabel;
    QLabel* m_controlOutlineDecorationLabel;
    QLabel* m_controlDoubleClickActionLabel;
    QCheckBox* m_showMouseoverOutlineCheckBox;
    QCheckBox* m_showClippedControlsCheckBox;
    QComboBox* m_blankControlDecorationBox;
    QComboBox* m_controlOutlineDecorationBox;
    QComboBox* m_controlDoubleClickActionBox;
};

#endif // SCENESETTINGSWIDGET_H