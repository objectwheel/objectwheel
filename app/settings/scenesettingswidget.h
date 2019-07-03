#ifndef SCENESETTINGSWIDGET_H
#define SCENESETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QComboBox;

class SceneSettingsWidget final : public SettingsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneSettingsWidget)

public:
    explicit SceneSettingsWidget(QWidget* parent = nullptr);
    void apply() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    void fill();

private:
    QGroupBox* m_designGroup;
    QLabel* m_showGuideLinesLabel;
    QLabel* m_sceneBackgroundColorLabel;
    QLabel* m_sceneZoomLevelLabel;
    QCheckBox* m_showGuideLinesCheckBox;
    QComboBox* m_sceneBackgroundColorBox;
    QComboBox* m_sceneZoomLevelBox;
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
    QLabel* m_controlOutlineLabel;
    QCheckBox* m_showMouseoverOutlineCheckBox;
    QComboBox* m_controlOutlineBox;
};

#endif // SCENESETTINGSWIDGET_H
