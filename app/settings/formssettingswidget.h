#ifndef FORMSSETTINGSWIDGET_H
#define FORMSSETTINGSWIDGET_H

#include <settingswidget.h>

class QLabel;
class QGroupBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QGridLayout;
class QVBoxLayout;
class QComboBox;

struct FormsSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit FormsSettingsWidget(QWidget* parent = nullptr);

    void apply() override;
    void reset() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString& word) const override;

private:
    QGroupBox* m_designGroup;
    QGridLayout* m_designLayout;
    QLabel* m_guidelinesLabel;
    QLabel* m_hoverOutlineLabel;
    QLabel* m_backgroundColorLabel;
    QLabel* m_outlineLabel;
    QLabel* m_zoomLevelLabel;
    QCheckBox* m_guidelinesCheckBox;
    QCheckBox* m_hoverOutlineCheckBox;
    QComboBox* m_backgroundColorBox;
    QComboBox* m_outlineBox;
    QComboBox* m_zoomLevelButton;
    /****/
    QGroupBox* m_gridViewGroup;
    QGridLayout* m_gridViewLayout;
    QLabel* m_visibilityLabel;
    QLabel* m_snappingLabel;
    QLabel* m_sizeLabel;
    QCheckBox* m_gridViewVisibleCheckBox;
    QCheckBox* m_snappingCheckBox;
    QSpinBox* m_sizeSpinBox;
};

#endif // FORMSSETTINGSWIDGET_H
