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
#include <QAction>

namespace {

const char* g_themes[] = {"Light"/*, "Dark"*/};
const char* g_bottomPanes[] = {"None", "Console Pane", "Issues Pane"};
const char* g_langIcons[] = {":/images/flags/en.png"};
const char* g_languages[] = {"English"};

void addThemes(QComboBox* comboBox)
{
    for (size_t i = 0; i < sizeof(g_themes) / sizeof(g_themes[0]); ++i)
        comboBox->addItem(QObject::tr(g_themes[i]), g_themes[i]);
}

void addBottomPanes(QComboBox* comboBox)
{
    for (size_t i = 0; i < sizeof(g_bottomPanes) / sizeof(g_bottomPanes[0]); ++i)
        comboBox->addItem(QObject::tr(g_bottomPanes[i]), g_bottomPanes[i]);
}

void addLanguages(QComboBox* comboBox)
{
    for (size_t i = 0; i < sizeof(g_languages) / sizeof(g_languages[0]); ++i)
        comboBox->addItem(QIcon(g_langIcons[i]), QObject::tr(g_languages[i]), g_languages[i]);
}
}

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
  //***
  , m_behavioralGroup(new QGroupBox(this))
  , m_behavioralLayout(new QGridLayout(m_behavioralGroup))
  , m_visibleBottomPaneLabel(new QLabel(m_behavioralGroup))
  , m_bottomPanesCheckBox(new QCheckBox(m_behavioralGroup))
  , m_visibleBottomPaneBox(new QComboBox(m_behavioralGroup))
{
    setTitle(tr("Interface"));

    m_layout->setSpacing(8);
    m_layout->setContentsMargins(6, 6, 6, 6);
    m_layout->addWidget(m_interfaceGroup);
    m_layout->addWidget(m_behavioralGroup);
    m_layout->addStretch();

    auto hb1 = new QHBoxLayout;
    hb1->setSpacing(8);
    hb1->setContentsMargins(0, 0, 0, 0);
    hb1->addWidget(m_topBarColorButton);
    hb1->addWidget(m_topBarColorResetButton);
    hb1->addStretch();
    auto hb2 = new QHBoxLayout;
    hb2->setSpacing(8);
    hb2->setContentsMargins(0, 0, 0, 0);
    hb2->addWidget(m_leftBarColorButton);
    hb2->addWidget(m_leftBarColorResetButton);
    hb2->addStretch();

    m_interfaceLayout->setSpacing(8);
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
    m_leftBarColorButton->setFixedWidth(64);
    m_topBarColorButton->setFixedWidth(64);

    m_topBarColorButton->setCursor(Qt::PointingHandCursor);
    m_leftBarColorButton->setCursor(Qt::PointingHandCursor);
    m_topBarColorResetButton->setCursor(Qt::PointingHandCursor);
    m_leftBarColorResetButton->setCursor(Qt::PointingHandCursor);
    m_themeBox->setCursor(Qt::PointingHandCursor);
    m_languageBox->setCursor(Qt::PointingHandCursor);
    m_hdpiCheckBox->setCursor(Qt::PointingHandCursor);

    m_behavioralLayout->setSpacing(8);
    m_behavioralLayout->setContentsMargins(6, 6, 6, 6);
    m_behavioralLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_behavioralLayout->addWidget(m_bottomPanesCheckBox, 0, 0);
    m_behavioralLayout->addWidget(m_visibleBottomPaneLabel, 0, 2);
    m_behavioralLayout->addWidget(m_visibleBottomPaneBox, 0, 3);
    m_behavioralLayout->setColumnStretch(4, 1);
    m_behavioralLayout->setColumnMinimumWidth(1, 20);

    m_behavioralGroup->setTitle(tr("Behavioral"));
    m_bottomPanesCheckBox->setText(tr("Pop up bottom pane when it flashes"));
    m_visibleBottomPaneLabel->setText(tr("Show bottom pane at startup") + ":");

    m_bottomPanesCheckBox->setToolTip(tr("Pop up bottom pane when it flashes"));
    m_visibleBottomPaneBox->setToolTip(tr("Bottom pane that will be open at startup by default"));

    m_bottomPanesCheckBox->setCursor(Qt::PointingHandCursor);
    m_visibleBottomPaneBox->setCursor(Qt::PointingHandCursor);

    addThemes(m_themeBox);
    addLanguages(m_languageBox);
    addBottomPanes(m_visibleBottomPaneBox);

    connect(m_leftBarColorResetButton, &QPushButton::clicked, this, [=] {
        m_leftBarColorButton->setColor(InterfaceSettings().leftBarColor);
    });
    connect(m_topBarColorResetButton, &QPushButton::clicked, this, [=] {
        m_topBarColorButton->setColor(InterfaceSettings().topBarColor);
    });
    connect(m_hdpiCheckBox, &QCheckBox::clicked, this, [=] {
        QMessageBox::information(this, tr("Restart Required"),
            tr("Be aware that the high DPI settings will take effect after application restart."));
    });
    connect(m_languageBox, qOverload<int>(&QComboBox::activated), this, [=] {
        QMessageBox::information(this, tr("Restart Required"),
            tr("Be aware that the language change will take effect after application restart."));
    });
    connect(m_themeBox, qOverload<int>(&QComboBox::activated), this, [=] {
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
    settings->theme = m_themeBox->currentData().toString();
    settings->language = m_languageBox->currentData().toString();
    settings->visibleBottomPane = m_visibleBottomPaneBox->currentData().toString();
    settings->hdpiEnabled = m_hdpiCheckBox->isChecked();
    settings->bottomPanesPop = m_bottomPanesCheckBox->isChecked();
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
    m_visibleBottomPaneBox->setCurrentText(tr(settings->visibleBottomPane.toUtf8()));
    m_hdpiCheckBox->setChecked(settings->hdpiEnabled);
    m_bottomPanesCheckBox->setChecked(settings->bottomPanesPop);
}