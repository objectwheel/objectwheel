#ifndef INTERFACESETTINGSWIDGET_H
#define INTERFACESETTINGSWIDGET_H

#include <settingswidget.h>

namespace Utils { class QtColorButton; }

class QLabel;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QGridLayout;
class QVBoxLayout;

class InterfaceSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit InterfaceSettingsWidget(QWidget* parent = nullptr);

    void apply() override;
    void reset() override;

private:
    QVBoxLayout* m_layout;
    QGroupBox* m_interfaceGroup;
    QGridLayout* m_interfaceLayout;
    QLabel* m_topBarColorLabel;
    QLabel* m_leftBarColorLabel;
    QLabel* m_themeLabel;
    QLabel* m_languageLabel;
    QLabel* m_hdpiLabel;
    Utils::QtColorButton* m_topBarColorButton;
    Utils::QtColorButton* m_leftBarColorButton;
    QPushButton* m_topBarColorResetButton;
    QPushButton* m_leftBarColorResetButton;
    QComboBox* m_themeBox;
    QComboBox* m_languageBox;
    QCheckBox* m_hdpiCheckBox;

    QGroupBox* m_behavioralGroup;
    QGridLayout* m_behavioralLayout;
    QLabel* m_visibleBottomPaneLabel;
    QCheckBox* m_bottomPanesCheckBox;
    QComboBox* m_visibleBottomPaneBox;
};

#endif // INTERFACESETTINGSWIDGET_H
