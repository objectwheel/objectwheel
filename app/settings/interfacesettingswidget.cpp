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
#include <QFontDatabase>

namespace {

bool g_themeMessageShowed = false;
bool g_languageMessageShowed = false;
bool g_hdpiMessageShowed = false;
bool g_fontMessageShowed = false;
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

bool containsWord(QComboBox* comboBox, const QString& word)
{
    for (int i = 0; i < comboBox->count(); ++i) {
        if (comboBox->itemText(i).contains(word, Qt::CaseInsensitive))
            return true;
    }
    return false;
}
}

InterfaceSettingsWidget::InterfaceSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_interfaceGroup(new QGroupBox(contentWidget()))
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
  /****/
  , m_fontGroup(new QGroupBox(contentWidget()))
  , m_fontLayout(new QVBoxLayout(m_fontGroup))
  , m_fontFamilyLabel(new QLabel(m_fontGroup))
  , m_fontSizeLabel(new QLabel(m_fontGroup))
  , m_fontFamilyBox(new QComboBox(m_fontGroup))
  , m_fontSizeBox(new QComboBox(m_fontGroup))
  , m_fontAntialiasingBox(new QCheckBox(m_fontGroup))
  , m_fontThickBox(new QCheckBox(m_fontGroup))
  , m_fontResetButton(new QPushButton(m_fontGroup))
  /****/
  , m_behavioralGroup(new QGroupBox(contentWidget()))
  , m_behavioralLayout(new QGridLayout(m_behavioralGroup))
  , m_visibleBottomPaneLabel(new QLabel(m_behavioralGroup))
  , m_bottomPanesCheckBox(new QCheckBox(m_behavioralGroup))
  , m_visibleBottomPaneBox(new QComboBox(m_behavioralGroup))
{
    contentLayout()->addWidget(m_interfaceGroup);
    contentLayout()->addWidget(m_fontGroup);
    contentLayout()->addWidget(m_behavioralGroup);
    contentLayout()->addStretch();

    /****/

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
    m_interfaceLayout->addWidget(m_topBarColorLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_leftBarColorLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_themeLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_languageLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_hdpiLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addLayout(hb1, 0, 2, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addLayout(hb2, 1, 2, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_themeBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_languageBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->addWidget(m_hdpiCheckBox, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_interfaceLayout->setColumnStretch(3, 1);
    m_interfaceLayout->setColumnMinimumWidth(1, 20);

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

    /****/

    auto hb3 = new QHBoxLayout;
    hb3->setSpacing(8);
    hb3->setContentsMargins(0, 0, 0, 0);
    hb3->addWidget(m_fontFamilyLabel);
    hb3->addWidget(m_fontFamilyBox);
    hb3->addSpacing(20);
    hb3->addWidget(m_fontSizeLabel);
    hb3->addWidget(m_fontSizeBox);
    hb3->addSpacing(30);
    hb3->addWidget(m_fontResetButton);
    hb3->addStretch();
    auto hb4 = new QHBoxLayout;
    hb4->setSpacing(8);
    hb4->setContentsMargins(0, 0, 0, 0);
    hb4->addWidget(m_fontAntialiasingBox);
    hb4->addWidget(m_fontThickBox);
    hb4->addStretch();

    m_fontLayout->setSpacing(8);
    m_fontLayout->setContentsMargins(6, 6, 6, 6);
    m_fontLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_fontLayout->addLayout(hb3);
    m_fontLayout->addLayout(hb4);

    m_fontGroup->setTitle(tr("Font"));
    m_fontFamilyLabel->setText(tr("Family") + ":");
    m_fontFamilyBox->addItems(QFontDatabase().families());
    m_fontSizeLabel->setText(tr("Size") + ":");
    m_fontSizeBox->addItems({"8", "9", "10", "11", "12", "13", "14", "15", "16",
                             "18", "24", "36", "48", "64", "72", "96", "144"});
    m_fontAntialiasingBox->setText(tr("Prefer antialiasing"));
    m_fontThickBox->setText(tr("Prefer thicker"));
    m_fontResetButton->setText(tr("Reset"));

    m_fontFamilyBox->setToolTip(tr("Chage font family"));
    m_fontSizeBox->setToolTip(tr("Chage font pixel size"));
    m_fontAntialiasingBox->setToolTip(tr("Enable font antialiasing"));
    m_fontThickBox->setToolTip(tr("Enable text thickness increasing"));
    m_fontResetButton->setToolTip(tr("Reset font settings to default"));

    m_fontFamilyBox->setCursor(Qt::PointingHandCursor);
    m_fontSizeBox->setCursor(Qt::PointingHandCursor);
    m_fontAntialiasingBox->setCursor(Qt::PointingHandCursor);
    m_fontThickBox->setCursor(Qt::PointingHandCursor);
    m_fontResetButton->setCursor(Qt::PointingHandCursor);

    /****/

    m_behavioralLayout->setSpacing(8);
    m_behavioralLayout->setContentsMargins(6, 6, 6, 6);
    m_behavioralLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_behavioralLayout->addWidget(m_bottomPanesCheckBox, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_behavioralLayout->addWidget(m_visibleBottomPaneLabel, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_behavioralLayout->addWidget(m_visibleBottomPaneBox, 0, 3, Qt::AlignLeft | Qt::AlignVCenter);
    m_behavioralLayout->setColumnStretch(4, 1);
    m_behavioralLayout->setColumnMinimumWidth(1, 20);

    m_behavioralGroup->setTitle(tr("Behavioral"));
    m_bottomPanesCheckBox->setText(tr("Pop up bottom pane when it flashes"));
    m_visibleBottomPaneLabel->setText(tr("Show bottom pane at startup") + ":");

    m_bottomPanesCheckBox->setToolTip(tr("Pop up bottom pane when it flashes"));
    m_visibleBottomPaneBox->setToolTip(tr("Bottom pane that will be open at startup by default"));

    m_bottomPanesCheckBox->setCursor(Qt::PointingHandCursor);
    m_visibleBottomPaneBox->setCursor(Qt::PointingHandCursor);

    /****/

    addThemes(m_themeBox);
    addLanguages(m_languageBox);
    addBottomPanes(m_visibleBottomPaneBox);

    connect(m_leftBarColorResetButton, &QPushButton::clicked, this, [=] {
        m_leftBarColorButton->setColor(InterfaceSettings().leftBarColor);
    });
    connect(m_topBarColorResetButton, &QPushButton::clicked, this, [=] {
        m_topBarColorButton->setColor(InterfaceSettings().topBarColor);
    });
    connect(m_fontResetButton, &QPushButton::clicked, this, [=] {
        const InterfaceSettings settings;
        m_fontFamilyBox->setCurrentText(settings.fontFamily);
        m_fontSizeBox->setCurrentText(QString::number(settings.fontPixelSize));
        m_fontThickBox->setChecked(settings.fontPreferThick);
        m_fontAntialiasingBox->setChecked(settings.fontPreferAntialiasing);
    });
    connect(m_fontAntialiasingBox, &QCheckBox::clicked, this, [=] {
        if (g_fontMessageShowed)
            return;
        g_fontMessageShowed = true;
        QMessageBox::information(this, tr("Restart Required"),
                                 tr("Be aware that the font settings will take effect after application restart."));
    });
    connect(m_fontThickBox, &QCheckBox::clicked, this, [=] {
        if (g_fontMessageShowed)
            return;
        g_fontMessageShowed = true;
        QMessageBox::information(this, tr("Restart Required"),
                                 tr("Be aware that the font settings will take effect after application restart."));
    });
    connect(m_fontSizeBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (g_fontMessageShowed)
            return;
        g_fontMessageShowed = true;
        QMessageBox::information(this, tr("Restart Required"),
                                 tr("Be aware that the font settings will take effect after application restart."));
    });
    connect(m_fontFamilyBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (g_fontMessageShowed)
            return;
        g_fontMessageShowed = true;
        QMessageBox::information(this, tr("Restart Required"),
                                 tr("Be aware that the font settings will take effect after application restart."));
    });
    connect(m_hdpiCheckBox, &QCheckBox::clicked, this, [=] {
        if (g_hdpiMessageShowed)
            return;
        g_hdpiMessageShowed = true;
        QMessageBox::information(this, tr("Restart Required"),
                                 tr("Be aware that the high DPI settings will take effect after application restart."));
    });
    connect(m_languageBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (g_languageMessageShowed)
            return;
        g_languageMessageShowed = true;
        QMessageBox::information(this, tr("Restart Required"),
                                 tr("Be aware that the language change will take effect after application restart."));
    });
    connect(m_themeBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (g_themeMessageShowed)
            return;
        g_themeMessageShowed = true;
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

    g_themeMessageShowed = false;
    g_languageMessageShowed = false;
    g_hdpiMessageShowed = false;
    g_fontMessageShowed = false;

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    /****/
    settings->topBarColor = m_topBarColorButton->color();
    settings->leftBarColor = m_leftBarColorButton->color();
    settings->theme = m_themeBox->currentData().toString();
    settings->language = m_languageBox->currentData().toString();
    settings->hdpiEnabled = m_hdpiCheckBox->isChecked();
    /****/
    settings->fontFamily = m_fontFamilyBox->currentText();
    settings->fontPixelSize = m_fontSizeBox->currentText().toInt();
    settings->fontPreferThick = m_fontThickBox->isChecked();
    settings->fontPreferAntialiasing = m_fontAntialiasingBox->isChecked();
    /****/
    settings->visibleBottomPane = m_visibleBottomPaneBox->currentData().toString();
    settings->bottomPanesPop = m_bottomPanesCheckBox->isChecked();
    /****/
    settings->write();
}

void InterfaceSettingsWidget::reset()
{
    if (!isActivated())
        return;

    activate(false);

    g_themeMessageShowed = false;
    g_languageMessageShowed = false;
    g_hdpiMessageShowed = false;
    g_fontMessageShowed = false;

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    /****/
    m_topBarColorButton->setColor(settings->topBarColor);
    m_leftBarColorButton->setColor(settings->leftBarColor);
    m_themeBox->setCurrentText(tr(settings->theme.toUtf8()));
    m_languageBox->setCurrentText(tr(settings->language.toUtf8()));
    m_hdpiCheckBox->setChecked(settings->hdpiEnabled);
    /****/
    m_fontFamilyBox->setCurrentText(settings->fontFamily);
    m_fontSizeBox->setCurrentText(QString::number(settings->fontPixelSize));
    m_fontThickBox->setChecked(settings->fontPreferThick);
    m_fontAntialiasingBox->setChecked(settings->fontPreferAntialiasing);
    /****/
    m_visibleBottomPaneBox->setCurrentText(tr(settings->visibleBottomPane.toUtf8()));
    m_bottomPanesCheckBox->setChecked(settings->bottomPanesPop);
}

QIcon InterfaceSettingsWidget::icon() const
{
    return QIcon(":/images/settings/interface.png");
}

QString InterfaceSettingsWidget::title() const
{
    return tr("Interface");
}

bool InterfaceSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_topBarColorLabel->text().contains(word, Qt::CaseInsensitive)
            || m_leftBarColorLabel->text().contains(word, Qt::CaseInsensitive)
            || m_themeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_languageLabel->text().contains(word, Qt::CaseInsensitive)
            || m_hdpiLabel->text().contains(word, Qt::CaseInsensitive)
            || m_hdpiCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_fontFamilyLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontAntialiasingBox->text().contains(word, Qt::CaseInsensitive)
            || m_fontThickBox->text().contains(word, Qt::CaseInsensitive)
            || m_visibleBottomPaneLabel->text().contains(word, Qt::CaseInsensitive)
            || m_bottomPanesCheckBox->text().contains(word, Qt::CaseInsensitive)
            || ::containsWord(m_themeBox, word)
            || ::containsWord(m_languageBox, word)
            || ::containsWord(m_visibleBottomPaneBox, word);
}