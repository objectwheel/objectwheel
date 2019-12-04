#include <interfacesettingswidget.h>
#include <interfacesettings.h>
#include <generalsettings.h>
#include <utilityfunctions.h>
#include <qtcolorbutton.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QFontDatabase>

InterfaceSettingsWidget::InterfaceSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_themeMessageShowed(false)
  , m_hdpiMessageShowed(false)
  , m_fontMessageShowed(false)
  , m_languageMessageShowed(false)
  /****/
  , m_interfaceGroup(new QGroupBox(contentWidget()))
  , m_themeLabel(new QLabel(m_interfaceGroup))
  , m_languageLabel(new QLabel(m_interfaceGroup))
  , m_hdpiLabel(new QLabel(m_interfaceGroup))
  , m_highlightColorLabel(new QLabel(m_interfaceGroup))
  , m_themeBox(new QComboBox(m_interfaceGroup))
  , m_languageBox(new QComboBox(m_interfaceGroup))
  , m_hdpiCheckBox(new QCheckBox(m_interfaceGroup))
  , m_highlightColorButton(new Utils::QtColorButton(m_interfaceGroup))
  , m_highlightColorResetButton(new QPushButton(m_interfaceGroup))
  /****/
  , m_fontGroup(new QGroupBox(contentWidget()))
  , m_fontFamilyLabel(new QLabel(m_fontGroup))
  , m_fontSizeLabel(new QLabel(m_fontGroup))
  , m_fontFamilyBox(new QComboBox(m_fontGroup))
  , m_fontSizeBox(new QComboBox(m_fontGroup))
  , m_fontAntialiasingBox(new QCheckBox(m_fontGroup))
  , m_fontThickBox(new QCheckBox(m_fontGroup))
  , m_fontResetButton(new QPushButton(m_fontGroup))
  /****/
  , m_behavioralGroup(new QGroupBox(contentWidget()))
  , m_outputPanePopsCheckBox(new QCheckBox(m_behavioralGroup))
  , m_preserveDesignerStateCheckBox(new QCheckBox(m_behavioralGroup))
  , m_designerStateResetButton(new QPushButton(m_behavioralGroup))
  , m_outputPaneMinimizedStartupCheckBox(new QCheckBox(m_behavioralGroup))
{
    contentLayout()->addWidget(m_interfaceGroup);
    contentLayout()->addWidget(m_fontGroup);
    contentLayout()->addWidget(m_behavioralGroup);
    contentLayout()->addStretch();

    /****/

    auto interfaceLayout = new QGridLayout(m_interfaceGroup);
    interfaceLayout->setSpacing(6);
    interfaceLayout->setContentsMargins(4, 4, 4, 4);
    interfaceLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    interfaceLayout->addWidget(m_themeLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_languageLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_hdpiLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_highlightColorLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_themeBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_languageBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_hdpiCheckBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_highlightColorButton, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    interfaceLayout->addWidget(m_highlightColorResetButton, 3, 2, Qt::AlignRight | Qt::AlignVCenter);
    interfaceLayout->setColumnStretch(3, 1);
    interfaceLayout->setColumnMinimumWidth(1, 20);

    m_interfaceGroup->setTitle(tr("User Interface"));
    m_hdpiCheckBox->setText(tr("Enable high DPI scaling"));
    m_themeLabel->setText(tr("Theme") + ":");
    m_languageLabel->setText(tr("Language") + ":");
    m_hdpiLabel->setText(tr("High DPI scaling") + ":");
    m_highlightColorLabel->setText(tr("Highlight color") + ":");
    m_highlightColorResetButton->setText(tr("Reset"));

    m_themeBox->setToolTip(tr("Change gui theme"));
    m_languageBox->setToolTip(tr("Change language"));
    m_hdpiCheckBox->setToolTip(tr("Enable high DPI scaling"));
    m_highlightColorButton->setToolTip(tr("Change highlight color"));
    m_highlightColorResetButton->setToolTip(tr("Reset highlight color to default"));

    m_themeBox->setCursor(Qt::PointingHandCursor);
    m_languageBox->setCursor(Qt::PointingHandCursor);
    m_hdpiCheckBox->setCursor(Qt::PointingHandCursor);
    m_highlightColorButton->setCursor(Qt::PointingHandCursor);
    m_highlightColorResetButton->setCursor(Qt::PointingHandCursor);

    m_highlightColorButton->setFixedWidth(64);

    /****/

    auto hb1 = new QHBoxLayout;
    hb1->setSpacing(6);
    hb1->setContentsMargins(0, 0, 0, 0);
    hb1->addWidget(m_fontFamilyLabel);
    hb1->addWidget(m_fontFamilyBox);
    hb1->addSpacing(20);
    hb1->addWidget(m_fontSizeLabel);
    hb1->addWidget(m_fontSizeBox);
    hb1->addSpacing(30);
    hb1->addWidget(m_fontResetButton);
    hb1->addStretch();

    auto hb2 = new QHBoxLayout;
    hb2->setSpacing(6);
    hb2->setContentsMargins(0, 0, 0, 0);
    hb2->addWidget(m_fontAntialiasingBox);
    hb2->addWidget(m_fontThickBox);
    hb2->addStretch();

    auto fontLayout = new QVBoxLayout(m_fontGroup);
    fontLayout->setSpacing(6);
    fontLayout->setContentsMargins(4, 4, 4, 4);
    fontLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    fontLayout->addLayout(hb1);
    fontLayout->addLayout(hb2);

    m_fontGroup->setTitle(tr("Font"));
    m_fontFamilyLabel->setText(tr("Family") + ":");
    m_fontSizeLabel->setText(tr("Size") + ":");
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

    auto behavioralLayout = new QGridLayout(m_behavioralGroup);
    behavioralLayout->setSpacing(6);
    behavioralLayout->setContentsMargins(4, 4, 4, 4);
    behavioralLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    behavioralLayout->addWidget(m_outputPanePopsCheckBox, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    behavioralLayout->addWidget(m_outputPaneMinimizedStartupCheckBox, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    behavioralLayout->addWidget(m_preserveDesignerStateCheckBox, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    behavioralLayout->addWidget(m_designerStateResetButton, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    behavioralLayout->setColumnStretch(4, 1);
    behavioralLayout->setColumnMinimumWidth(1, 20);

    m_behavioralGroup->setTitle(tr("Behavioral"));
    m_outputPanePopsCheckBox->setText(tr("Pop up output pane when it flashes"));
    m_preserveDesignerStateCheckBox->setText(tr("Enable interface state saving"));
    m_designerStateResetButton->setText(tr("Reset interface state"));
    m_outputPaneMinimizedStartupCheckBox->setText(tr("Enable minimized output pane startup"));

    m_outputPanePopsCheckBox->setToolTip(tr("Pop up output pane when it flashes (ie. when a "
                                            "console print happens)"));
    m_preserveDesignerStateCheckBox->setToolTip(tr("Enabling this option leads preserving tool bars, dock widgets, pane "
                                                   "postions and other designer states between application starts"));
    m_designerStateResetButton->setToolTip(tr("Resets tool bars, dock widgets, pane postions and other interface "
                                              "states to default"));
    m_outputPaneMinimizedStartupCheckBox->setToolTip(tr("Do not pop up the output pane at startup even "
                                                        "if it is left open in previous session"));

    m_outputPanePopsCheckBox->setCursor(Qt::PointingHandCursor);
    m_designerStateResetButton->setCursor(Qt::PointingHandCursor);
    m_preserveDesignerStateCheckBox->setCursor(Qt::PointingHandCursor);
    m_outputPaneMinimizedStartupCheckBox->setCursor(Qt::PointingHandCursor);

    /****/

    fill();

    connect(m_highlightColorResetButton, &QPushButton::clicked, this, [=] {
        m_highlightColorButton->setColor(InterfaceSettings(0).highlightColor);
    });
    connect(m_fontResetButton, &QPushButton::clicked, this, [=] {
        const InterfaceSettings settings(0);
        m_fontFamilyBox->setCurrentText(settings.fontFamily);
        m_fontSizeBox->setCurrentText(QString::number(settings.fontPixelSize));
        m_fontThickBox->setChecked(settings.fontPreferThick);
        m_fontAntialiasingBox->setChecked(settings.fontPreferAntialiasing);
    });
    connect(m_fontAntialiasingBox, &QCheckBox::clicked, this, [=] {
        if (m_fontMessageShowed)
            return;
        m_fontMessageShowed = true;
        UtilityFunctions::showMessage(
                    this, tr("Restart required"),
                    tr("Be aware that the font settings will take effect after application restart."),
                    QMessageBox::Information);
    });
    connect(m_fontThickBox, &QCheckBox::clicked, this, [=] {
        if (m_fontMessageShowed)
            return;
        m_fontMessageShowed = true;
        UtilityFunctions::showMessage(
                    this, tr("Restart required"),
                    tr("Be aware that the font settings will take effect after application restart."),
                    QMessageBox::Information);
    });
    connect(m_fontSizeBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (m_fontMessageShowed)
            return;
        m_fontMessageShowed = true;
        UtilityFunctions::showMessage(
                    this, tr("Restart required"),
                    tr("Be aware that the font settings will take effect after application restart."),
                    QMessageBox::Information);
    });
    connect(m_fontFamilyBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (m_fontMessageShowed)
            return;
        m_fontMessageShowed = true;
        UtilityFunctions::showMessage(
                    this, tr("Restart required"),
                    tr("Be aware that the font settings will take effect after application restart."),
                    QMessageBox::Information);
    });
    connect(m_hdpiCheckBox, &QCheckBox::clicked, this, [=] {
        if (m_hdpiMessageShowed)
            return;
        m_hdpiMessageShowed = true;
        UtilityFunctions::showMessage(
                    this, tr("Restart required"),
                    tr("Be aware that the high DPI settings will take effect after application restart."),
                    QMessageBox::Information);
    });
    connect(m_languageBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (m_languageMessageShowed)
            return;
        m_languageMessageShowed = true;
        UtilityFunctions::showMessage(
                    this, tr("Restart required"),
                    tr("Be aware that the language change will take effect after application restart."),
                    QMessageBox::Information);
    });
    connect(m_themeBox, qOverload<int>(&QComboBox::activated), this, [=] {
        if (m_themeMessageShowed)
            return;
        m_themeMessageShowed = true;
        UtilityFunctions::showMessage(
                    this, tr("Restart required"),
                    tr("Be aware that the theme change will take effect after application restart."),
                    QMessageBox::Information);
    });
    connect(m_designerStateResetButton, &QCheckBox::clicked, this, [=] {
        int ret = UtilityFunctions::showMessage(
                    this, tr("Are you sure?"),
                    tr("This will reset tool bars, dock widgets, pane postions and other designer "
                       "states to defaults. Are you sure to proceed?"),
                    QMessageBox::Question, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::Yes)
            emit GeneralSettings::instance()->designerStateReset();
    });

    activate();
    revert();
}

void InterfaceSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    m_themeMessageShowed = false;
    m_hdpiMessageShowed = false;
    m_fontMessageShowed = false;
    m_languageMessageShowed = false;

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    /****/
    settings->theme = m_themeBox->currentIndex();
    settings->language = m_languageBox->currentIndex();
    settings->hdpiEnabled = m_hdpiCheckBox->isChecked();
    settings->highlightColor = m_highlightColorButton->color();
    /****/
    settings->fontFamily = m_fontFamilyBox->currentText();
    settings->fontPixelSize = m_fontSizeBox->currentText().toInt();
    settings->fontPreferThick = m_fontThickBox->isChecked();
    settings->fontPreferAntialiasing = m_fontAntialiasingBox->isChecked();
    /****/
    settings->outputPanePops = m_outputPanePopsCheckBox->isChecked();
    settings->preserveDesignerState = m_preserveDesignerStateCheckBox->isChecked();
    settings->outputPaneMinimizedStartupEnabled = m_outputPaneMinimizedStartupCheckBox->isChecked();
    /****/
    settings->write();
}

void InterfaceSettingsWidget::revert()
{
    if (!isActivated())
        return;

    activate(false);

    m_themeMessageShowed = false;
    m_hdpiMessageShowed = false;
    m_fontMessageShowed = false;
    m_languageMessageShowed = false;

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    /****/
    m_themeBox->setCurrentIndex(settings->theme);
    m_languageBox->setCurrentIndex(settings->language);
    m_hdpiCheckBox->setChecked(settings->hdpiEnabled);
    m_highlightColorButton->setColor(settings->highlightColor);
    /****/
    m_fontFamilyBox->setCurrentText(settings->fontFamily);
    m_fontSizeBox->setCurrentText(QString::number(settings->fontPixelSize));
    m_fontThickBox->setChecked(settings->fontPreferThick);
    m_fontAntialiasingBox->setChecked(settings->fontPreferAntialiasing);
    /****/
    m_outputPanePopsCheckBox->setChecked(settings->outputPanePops);
    m_preserveDesignerStateCheckBox->setChecked(settings->preserveDesignerState);
    m_outputPaneMinimizedStartupCheckBox->setChecked(settings->outputPaneMinimizedStartupEnabled);
}

void InterfaceSettingsWidget::reset()
{
    GeneralSettings::interfaceSettings()->reset();
    GeneralSettings::interfaceSettings()->write();
    activate();
    revert();
    emit GeneralSettings::instance()->designerStateReset();
}

QIcon InterfaceSettingsWidget::icon() const
{
    return QIcon(":/images/settings/interface.svg");
}

QString InterfaceSettingsWidget::title() const
{
    return tr("Interface");
}

bool InterfaceSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_fontGroup->title().contains(word, Qt::CaseInsensitive)
            || m_behavioralGroup->title().contains(word, Qt::CaseInsensitive)
            || m_themeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_languageLabel->text().contains(word, Qt::CaseInsensitive)
            || m_hdpiLabel->text().contains(word, Qt::CaseInsensitive)
            || m_highlightColorLabel->text().contains(word, Qt::CaseInsensitive)
            || m_hdpiCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_fontFamilyLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontAntialiasingBox->text().contains(word, Qt::CaseInsensitive)
            || m_fontThickBox->text().contains(word, Qt::CaseInsensitive)
            || m_outputPanePopsCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_preserveDesignerStateCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_outputPaneMinimizedStartupCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_preserveDesignerStateCheckBox->toolTip().contains(word, Qt::CaseInsensitive)
            || m_designerStateResetButton->text().contains(word, Qt::CaseInsensitive)
            || m_designerStateResetButton->toolTip().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_themeBox, word)
            || UtilityFunctions::comboContainsWord(m_languageBox, word);
}

void InterfaceSettingsWidget::fill()
{
    m_themeBox->addItem(tr("Light"));
    m_languageBox->addItem(QIcon(":/images/flags/en.svg"), tr("English"));
    m_fontFamilyBox->addItems(QFontDatabase().families());
    m_fontSizeBox->addItems({"8", "9", "10", "11", "12", "13", "14", "15", "16",
                             "18", "24", "36", "48", "64", "72", "96", "144"});
}