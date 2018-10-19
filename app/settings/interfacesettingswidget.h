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
    void reject() override;
    QString title() const override;

private:
    QVBoxLayout* m_layout;
    QGroupBox* m_interfaceGroup;
    QGridLayout* m_interfaceLayout;
    QLabel* m_colorLabel;
    QLabel* m_themeLabel;
    QLabel* m_languageLabel;
    QLabel* m_hdpiLabel;
    Utils::QtColorButton* m_colorButton;
    QPushButton* m_colorResetButton;
    QComboBox* m_themeBox;
    QComboBox* m_languageBox;
    QCheckBox* m_hdpiCheckBox;
};

#endif // INTERFACESETTINGSWIDGET_H
