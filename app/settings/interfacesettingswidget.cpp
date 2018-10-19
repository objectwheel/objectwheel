#include <interfacesettingswidget.h>
#include <interfacesettings.h>
#include <generalsettings.h>
#include <qtcolorbutton.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>

InterfaceSettingsWidget::InterfaceSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_interfaceGroup(new QGroupBox(this))
  , m_interfaceLayout(new QGridLayout(m_interfaceGroup))
  , m_colorLabel(new QLabel(m_interfaceGroup))
  , m_themeLabel(new QLabel(m_interfaceGroup))
  , m_languageLabel(new QLabel(m_interfaceGroup))
  , m_hdpiLabel(new QLabel(m_interfaceGroup))
  , m_colorButton(new Utils::QtColorButton(m_interfaceGroup))
  , m_colorResetButton(new QPushButton(m_interfaceGroup))
  , m_themeBox(new QComboBox(m_interfaceGroup))
  , m_languageBox(new QComboBox(m_interfaceGroup))
  , m_hdpiCheckBox(new QCheckBox(m_interfaceGroup))
{
    m_layout->setSpacing(6);
    m_layout->setContentsMargins(6, 6, 6, 6);
    m_layout->addWidget(m_interfaceGroup);
    m_layout->addStretch();

    m_interfaceLayout->setSpacing(6);
    m_interfaceLayout->setContentsMargins(6, 6, 6, 6);

    m_interfaceLayout->addWidget(m_colorLabel, 0, 0);
    m_interfaceLayout->addWidget(m_themeLabel, 1, 0);
    m_interfaceLayout->addWidget(m_languageLabel, 2, 0);
    m_interfaceLayout->addWidget(m_hdpiLabel, 3, 0);

    m_interfaceLayout->addWidget(m_colorButton, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_colorResetButton, 0, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    m_interfaceLayout->addWidget(m_themeBox, 1, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    m_interfaceLayout->addWidget(m_languageBox, 2, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    m_interfaceLayout->addWidget(m_hdpiCheckBox, 3, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    m_interfaceLayout->setColumnStretch(3, 1);

    m_interfaceGroup->setTitle(tr("User Interface"));
    m_colorResetButton->setText(tr("Reset"));
    m_hdpiCheckBox->setText(tr("Enable high DPI scaling"));
    m_colorLabel->setText(tr("Color") + ":");
    m_themeLabel->setText(tr("Theme") + ":");
    m_languageLabel->setText(tr("Language") + ":");
    m_hdpiLabel->setText(tr("High DPI scaling") + ":");

    m_colorButton->setToolTip(tr("Chage the color of page switcher bar"));
    m_colorResetButton->setToolTip(tr("Reset color to default"));
    m_themeBox->setToolTip(tr("Change gui theme"));
    m_languageBox->setToolTip(tr("Change language"));
    m_hdpiCheckBox->setToolTip(tr("Enable high DPI scaling"));

    m_colorButton->setCursor(Qt::PointingHandCursor);
    m_colorResetButton->setCursor(Qt::PointingHandCursor);
    m_themeBox->setCursor(Qt::PointingHandCursor);
    m_languageBox->setCursor(Qt::PointingHandCursor);
    m_hdpiCheckBox->setCursor(Qt::PointingHandCursor);

    m_colorButton->setFixedWidth(64);
    m_themeBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_languageBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

void InterfaceSettingsWidget::apply()
{
    if (!isActivated())
        return;

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->color = m_colorButton->color();
    settings->theme = m_themeBox->currentText();
    settings->language = m_languageBox->currentText();
    settings->hdpiEnabled = m_hdpiCheckBox->isChecked();
}

void InterfaceSettingsWidget::clean()
{
    if (!isActivated())
        return;

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    m_colorButton->setColor(settings->color);
    m_themeBox->setCurrentText(tr(settings->theme.toUtf8()));
    m_languageBox->setCurrentText(tr(settings->language.toUtf8()));
    m_hdpiCheckBox->setChecked(settings->hdpiEnabled);
}

QString InterfaceSettingsWidget::title() const
{
    return tr("Interface");
}
