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
#include <QMessageBox>

InterfaceSettingsWidget::InterfaceSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_interfaceGroup(new QGroupBox(this))
  , m_interfaceLayout(new QGridLayout(m_interfaceGroup))
  , m_topBarColorLabel(new QLabel(m_interfaceGroup))
  , m_leftBarColorLabel(new QLabel(m_interfaceGroup))
  , m_themeLabel(new QLabel(m_interfaceGroup))
  , m_languageLabel(new QLabel(m_interfaceGroup))
  , m_hdpiLabel(new QLabel(m_interfaceGroup))
  , m_topBarColorButton(new Utils::QtColorButton(m_interfaceGroup))
  , m_leftBarColorButton(new Utils::QtColorButton(m_interfaceGroup))
  , m_topBarColorResetButton(new QPushButton(m_interfaceGroup))
  , m_leftBarColorResetButton(new QPushButton(m_interfaceGroup))
  , m_themeBox(new QComboBox(m_interfaceGroup))
  , m_languageBox(new QComboBox(m_interfaceGroup))
  , m_hdpiCheckBox(new QCheckBox(m_interfaceGroup))
{
    m_layout->setSpacing(6);
    m_layout->setContentsMargins(6, 6, 6, 6);
    m_layout->addWidget(m_interfaceGroup);
    m_layout->addStretch();

    auto hb1 = new QHBoxLayout;
    hb1->setSpacing(6);
    hb1->setContentsMargins(0, 0, 0, 0);
    hb1->addWidget(m_topBarColorButton);
    hb1->addWidget(m_topBarColorResetButton);
    hb1->addStretch();
    auto hb2 = new QHBoxLayout;
    hb2->setSpacing(6);
    hb2->setContentsMargins(0, 0, 0, 0);
    hb2->addWidget(m_leftBarColorButton);
    hb2->addWidget(m_leftBarColorResetButton);
    hb2->addStretch();

    m_interfaceLayout->setSpacing(6);
    m_interfaceLayout->setContentsMargins(6, 6, 6, 6);
    m_interfaceLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_interfaceLayout->addWidget(m_topBarColorLabel, 0, 0);
    m_interfaceLayout->addWidget(m_leftBarColorLabel, 1, 0);
    m_interfaceLayout->addWidget(m_themeLabel, 2, 0);
    m_interfaceLayout->addWidget(m_languageLabel, 3, 0);
    m_interfaceLayout->addWidget(m_hdpiLabel, 4, 0);
    m_interfaceLayout->addLayout(hb1, 0, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addLayout(hb2, 1, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_themeBox, 2, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_languageBox, 3, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_hdpiCheckBox, 4, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->setColumnStretch(3, 1);

    m_interfaceGroup->setTitle(tr("User Interface"));
    m_topBarColorResetButton->setText(tr("Reset"));
    m_leftBarColorResetButton->setText(tr("Reset"));
    m_hdpiCheckBox->setText(tr("Enable high DPI scaling"));
    m_topBarColorLabel->setText(tr("Top bar color") + ":");
    m_leftBarColorLabel->setText(tr("Left bar color") + ":");
    m_themeLabel->setText(tr("Theme") + ":");
    m_languageLabel->setText(tr("Language") + ":");
    m_hdpiLabel->setText(tr("High DPI scaling") + ":");

    m_topBarColorResetButton->setToolTip(tr("Reset color to default"));
    m_leftBarColorResetButton->setToolTip(tr("Reset color to default"));
    m_topBarColorButton->setToolTip(tr("Chage top bar color"));
    m_leftBarColorButton->setToolTip(tr("Chage left bar color"));
    m_themeBox->setToolTip(tr("Change gui theme"));
    m_languageBox->setToolTip(tr("Change language"));
    m_hdpiCheckBox->setToolTip(tr("Enable high DPI scaling"));

    m_topBarColorButton->setFixedWidth(64);
    m_leftBarColorButton->setFixedWidth(64);

    m_topBarColorButton->setCursor(Qt::PointingHandCursor);
    m_leftBarColorButton->setCursor(Qt::PointingHandCursor);
    m_topBarColorResetButton->setCursor(Qt::PointingHandCursor);
    m_leftBarColorResetButton->setCursor(Qt::PointingHandCursor);
    m_themeBox->setCursor(Qt::PointingHandCursor);
    m_languageBox->setCursor(Qt::PointingHandCursor);
    m_hdpiCheckBox->setCursor(Qt::PointingHandCursor);

    m_themeBox->addItem(tr("Light"));
    m_languageBox->addItem(tr("English"));

    connect(m_leftBarColorResetButton, &QPushButton::clicked, this, [=] {
        m_leftBarColorButton->setColor(InterfaceSettings().leftBarColor);
    });
    connect(m_topBarColorResetButton, &QPushButton::clicked, this, [=] {
        m_topBarColorButton->setColor(InterfaceSettings().topBarColor);
    });
    connect(m_hdpiCheckBox, &QCheckBox::toggled, this, [=] {
        QMessageBox::information(this, tr("Restart Required"),
            tr("Be aware that the high DPI settings will take effect after application restart."));
    });
    connect(m_languageBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=] {
        QMessageBox::information(this, tr("Restart Required"),
            tr("Be aware that the language change will take effect after application restart."));
    });
    connect(m_themeBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=] {
        QMessageBox::information(this, tr("Restart Required"),
            tr("Be aware that the theme change will take effect after application restart."));
    });

    activate();
    reset();
}

void InterfaceSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->topBarColor = m_topBarColorButton->color();
    settings->leftBarColor = m_leftBarColorButton->color();
    settings->theme = m_themeBox->currentText();
    settings->language = m_languageBox->currentText();
    settings->hdpiEnabled = m_hdpiCheckBox->isChecked();
    settings->write();
}

void InterfaceSettingsWidget::reset()
{
    if (!isActivated())
        return;

    activate(false);

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    m_topBarColorButton->setColor(settings->topBarColor);
    m_leftBarColorButton->setColor(settings->leftBarColor);
    m_themeBox->setCurrentText(tr(settings->theme.toUtf8()));
    m_languageBox->setCurrentText(tr(settings->language.toUtf8()));
    m_hdpiCheckBox->setChecked(settings->hdpiEnabled);
}

QString InterfaceSettingsWidget::title() const
{
    return tr("Interface");
}
