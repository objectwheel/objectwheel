#include <formssettingswidget.h>
#include <formssettings.h>
#include <designersettings.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QAction>
#include <QFontDatabase>
#include <QSpinBox>

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

FormsSettingsWidget::FormsSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_designGroup(new QGroupBox(contentWidget()))
  , m_designLayout(new QGridLayout(m_designGroup))
  , m_guidelinesLabel(new QLabel(m_designGroup))
  , m_hoverOutlineLabel(new QLabel(m_designGroup))
  , m_backgroundColorLabel(new QLabel(m_designGroup))
  , m_outlineLabel(new QLabel(m_designGroup))
  , m_zoomLevelLabel(new QLabel(m_designGroup))
  , m_guidelinesCheckBox(new QCheckBox(m_designGroup))
  , m_hoverOutlineCheckBox(new QCheckBox(m_designGroup))
  , m_backgroundColorBox(new QComboBox(m_designGroup))
  , m_outlineBox(new QComboBox(m_designGroup))
  , m_zoomLevelButton(new QComboBox(m_designGroup))
  /****/
  , m_gridViewGroup(new QGroupBox(contentWidget()))
  , m_gridViewLayout(new QGridLayout(m_gridViewGroup))
  , m_visibilityLabel(new QLabel(m_gridViewGroup))
  , m_snappingLabel(new QLabel(m_gridViewGroup))
  , m_sizeLabel(new QLabel(m_gridViewGroup))
  , m_gridViewVisibleCheckBox(new QCheckBox(m_gridViewGroup))
  , m_snappingCheckBox(new QCheckBox(m_gridViewGroup))
  , m_sizeSpinBox(new QSpinBox(m_gridViewGroup))
{
    contentLayout()->addWidget(m_designGroup);
    contentLayout()->addWidget(m_gridViewGroup);
    contentLayout()->addStretch();

    /****/

    m_designLayout->setSpacing(8);
    m_designLayout->setContentsMargins(6, 6, 6, 6);
    m_designLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_designLayout->addWidget(m_guidelinesLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_hoverOutlineLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_backgroundColorLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_outlineLabel, 5, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_zoomLevelLabel, 6, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_guidelinesCheckBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_hoverOutlineCheckBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_backgroundColorBox, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_outlineBox, 5, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_zoomLevelButton, 6, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->setColumnStretch(3, 1);
    m_designLayout->setColumnMinimumWidth(1, 20);

    m_designGroup->setTitle(tr("Design"));
    m_guidelinesCheckBox->setText(tr("Visible"));
    m_hoverOutlineCheckBox->setText(tr("Visible"));
    m_guidelinesLabel->setText(tr("Guidelines") + ":");
    m_hoverOutlineLabel->setText(tr("Hover outline") + ":");
    m_backgroundColorLabel->setText(tr("Background color") + ":");
    m_outlineLabel->setText(tr("Outline") + ":");
    m_zoomLevelLabel->setText(tr("Zoom level") + ":");

    m_guidelinesCheckBox->setToolTip(tr("Show guidelines"));
    m_hoverOutlineCheckBox->setToolTip(tr("Show outline for mouse hover on controls"));
    m_backgroundColorBox->setToolTip(tr("Change default background color of forms"));
    m_outlineBox->setToolTip(tr("Change outline mode for controls"));
    m_zoomLevelButton->setToolTip(tr("Change zoom level of designer"));

    m_guidelinesCheckBox->setCursor(Qt::PointingHandCursor);
    m_hoverOutlineCheckBox->setCursor(Qt::PointingHandCursor);
    m_backgroundColorBox->setCursor(Qt::PointingHandCursor);
    m_outlineBox->setCursor(Qt::PointingHandCursor);
    m_zoomLevelButton->setCursor(Qt::PointingHandCursor);

//    /****/

//    auto hb3 = new QHBoxLayout;
//    hb3->setSpacing(8);
//    hb3->setContentsMargins(0, 0, 0, 0);
//    hb3->addWidget(m_fontFamilyLabel);
//    hb3->addWidget(m_fontFamilyBox);
//    hb3->addSpacing(20);
//    hb3->addWidget(m_fontSizeLabel);
//    hb3->addWidget(m_fontSizeBox);
//    hb3->addSpacing(30);
//    hb3->addWidget(m_fontResetButton);
//    hb3->addStretch();
//    auto hb4 = new QHBoxLayout;
//    hb4->setSpacing(8);
//    hb4->setContentsMargins(0, 0, 0, 0);
//    hb4->addWidget(m_fontAntialiasingBox);
//    hb4->addWidget(m_fontThickBox);
//    hb4->addStretch();

//    m_fontLayout->setSpacing(8);
//    m_fontLayout->setContentsMargins(6, 6, 6, 6);
//    m_fontLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//    m_fontLayout->addLayout(hb3);
//    m_fontLayout->addLayout(hb4);

//    m_fontGroup->setTitle(tr("Font"));
//    m_fontFamilyLabel->setText(tr("Family") + ":");
//    m_fontFamilyBox->addItems(QFontDatabase().families());
//    m_fontSizeLabel->setText(tr("Size") + ":");
//    m_fontSizeBox->addItems({"8", "9", "10", "11", "12", "13", "14", "15", "16",
//                             "18", "24", "36", "48", "64", "72", "96", "144"});
//    m_fontAntialiasingBox->setText(tr("Prefer antialiasing"));
//    m_fontThickBox->setText(tr("Prefer thicker"));
//    m_fontResetButton->setText(tr("Reset"));

//    m_fontFamilyBox->setToolTip(tr("Chage font family"));
//    m_fontSizeBox->setToolTip(tr("Chage font pixel size"));
//    m_fontAntialiasingBox->setToolTip(tr("Enable font antialiasing"));
//    m_fontThickBox->setToolTip(tr("Enable text thickness increasing"));
//    m_fontResetButton->setToolTip(tr("Reset font settings to default"));

//    m_fontFamilyBox->setCursor(Qt::PointingHandCursor);
//    m_fontSizeBox->setCursor(Qt::PointingHandCursor);
//    m_fontAntialiasingBox->setCursor(Qt::PointingHandCursor);
//    m_fontThickBox->setCursor(Qt::PointingHandCursor);
//    m_fontResetButton->setCursor(Qt::PointingHandCursor);

//    /****/

//    m_behavioralLayout->setSpacing(8);
//    m_behavioralLayout->setContentsMargins(6, 6, 6, 6);
//    m_behavioralLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//    m_behavioralLayout->addWidget(m_bottomPanesCheckBox, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
//    m_behavioralLayout->addWidget(m_preserveDesignerStateCheckBox, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
//    m_behavioralLayout->addWidget(m_visibleBottomPaneLabel, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
//    m_behavioralLayout->addWidget(m_visibleBottomPaneBox, 0, 3, Qt::AlignLeft | Qt::AlignVCenter);
//    m_behavioralLayout->addWidget(m_designerStateResetButton, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
//    m_behavioralLayout->setColumnStretch(4, 1);
//    m_behavioralLayout->setColumnMinimumWidth(1, 20);

//    m_behavioralGroup->setTitle(tr("Behavioral"));
//    m_bottomPanesCheckBox->setText(tr("Pop up bottom pane when it flashes"));
//    m_preserveDesignerStateCheckBox->setText(tr("Enable designer state saving"));
//    m_designerStateResetButton->setText(tr("Reset designer states to default"));
//    m_visibleBottomPaneLabel->setText(tr("Show bottom pane at startup") + ":");

//    m_bottomPanesCheckBox->setToolTip(tr("Pop up bottom pane when it flashes"));
//    m_preserveDesignerStateCheckBox->setToolTip(tr("Enabling this option leads preserving tool bars, dock widgets, pane "
//                                                   "postions and other designer states between application starts"));
//    m_designerStateResetButton->setToolTip(tr("Resets tool bars, dock widgets, pane postions and other designer "
//                                             "states to default"));
//    m_visibleBottomPaneBox->setToolTip(tr("Bottom pane that will be open at startup by default"));

//    m_bottomPanesCheckBox->setCursor(Qt::PointingHandCursor);
//    m_designerStateResetButton->setCursor(Qt::PointingHandCursor);
//    m_preserveDesignerStateCheckBox->setCursor(Qt::PointingHandCursor);
//    m_visibleBottomPaneBox->setCursor(Qt::PointingHandCursor);

//    /****/

//    addThemes(m_themeBox);
//    addLanguages(m_languageBox);
//    addBottomPanes(m_visibleBottomPaneBox);

//    connect(m_fontResetButton, &QPushButton::clicked, this, [=] {
//        const FormsSettings settings;
//        m_fontFamilyBox->setCurrentText(settings.fontFamily);
//        m_fontSizeBox->setCurrentText(QString::number(settings.fontPixelSize));
//        m_fontThickBox->setChecked(settings.fontPreferThick);
//        m_fontAntialiasingBox->setChecked(settings.fontPreferAntialiasing);
//    });
//    connect(m_fontAntialiasingBox, &QCheckBox::clicked, this, [=] {
//        if (g_fontMessageShowed)
//            return;
//        g_fontMessageShowed = true;
//        UtilityFunctions::showMessage(
//                    this, tr("Restart required"),
//                    tr("Be aware that the font settings will take effect after application restart."),
//                    QMessageBox::Information);
//    });
//    connect(m_fontThickBox, &QCheckBox::clicked, this, [=] {
//        if (g_fontMessageShowed)
//            return;
//        g_fontMessageShowed = true;
//        UtilityFunctions::showMessage(
//                    this, tr("Restart required"),
//                    tr("Be aware that the font settings will take effect after application restart."),
//                    QMessageBox::Information);
//    });
//    connect(m_fontSizeBox, qOverload<int>(&QComboBox::activated), this, [=] {
//        if (g_fontMessageShowed)
//            return;
//        g_fontMessageShowed = true;
//        UtilityFunctions::showMessage(
//                    this, tr("Restart required"),
//                    tr("Be aware that the font settings will take effect after application restart."),
//                    QMessageBox::Information);
//    });
//    connect(m_fontFamilyBox, qOverload<int>(&QComboBox::activated), this, [=] {
//        if (g_fontMessageShowed)
//            return;
//        g_fontMessageShowed = true;
//        UtilityFunctions::showMessage(
//                    this, tr("Restart required"),
//                    tr("Be aware that the font settings will take effect after application restart."),
//                    QMessageBox::Information);
//    });
//    connect(m_hdpiCheckBox, &QCheckBox::clicked, this, [=] {
//        if (g_hdpiMessageShowed)
//            return;
//        g_hdpiMessageShowed = true;
//        UtilityFunctions::showMessage(
//                    this, tr("Restart required"),
//                    tr("Be aware that the high DPI settings will take effect after application restart."),
//                    QMessageBox::Information);
//    });
//    connect(m_languageBox, qOverload<int>(&QComboBox::activated), this, [=] {
//        if (g_languageMessageShowed)
//            return;
//        g_languageMessageShowed = true;
//        UtilityFunctions::showMessage(
//                    this, tr("Restart required"),
//                    tr("Be aware that the language change will take effect after application restart."),
//                    QMessageBox::Information);
//    });
//    connect(m_themeBox, qOverload<int>(&QComboBox::activated), this, [=] {
//        if (g_themeMessageShowed)
//            return;
//        g_themeMessageShowed = true;
//        UtilityFunctions::showMessage(
//                    this, tr("Restart required"),
//                    tr("Be aware that the theme change will take effect after application restart."),
//                    QMessageBox::Information);
//    });
//    connect(m_designerStateResetButton, &QCheckBox::clicked, this, [=] {
//        int ret = UtilityFunctions::showMessage(
//                    this, tr("Are you sure?"),
//                    tr("This will reset tool bars, dock widgets, pane postions and other designer "
//                       "states to defaults. Are you sure to proceed?"),
//                    QMessageBox::Question, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
////   FIXME     if (ret == QMessageBox::Yes)
////            emit DesignerSettings::instance()->designerStateReset();
//    });

    activate();
    reset();
}

void FormsSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    FormsSettings* settings = DesignerSettings::formsSettings();
    /****/
    settings->guidelinesVisible = m_guidelinesCheckBox->currentData().toString();
    settings->hoverOutlineVisible = m_hoverOutlineCheckBox->currentData().toString();
    settings->backgroundColorMode = m_backgroundColorBox->isChecked();
    settings->outlineMode = m_outlineBox->isChecked();
    settings->zoomLevel = m_zoomLevelButton->isChecked();
    /****/
    settings->gridViewVisible = m_gridViewVisibleCheckBox->currentText().toInt();
    settings->snappingEnabled = m_snappingCheckBox->isChecked();
    settings->gridSize = m_sizeSpinBox->isChecked();
    /****/
    settings->write();
}

void FormsSettingsWidget::reset()
{
    if (!isActivated())
        return;

    activate(false);

    const FormsSettings* settings = DesignerSettings::formsSettings();
    /****/
    m_guidelinesCheckBox->setChecked(settings->guidelinesVisible);
    m_hoverOutlineCheckBox->setChecked(settings->hoverOutlineVisible);
    m_backgroundColorBox->setCurrentText(tr(settings->backgroundColorMode.toUtf8()));
    m_outlineBox->setCurrentText(tr(settings->outlineMode.toUtf8()));
    m_zoomLevelButton->setCurrentText(tr(settings->zoomLevel.toUtf8()));
    /****/
    m_gridViewVisibleCheckBox->setChecked(settings->gridViewVisible);
    m_snappingCheckBox->setChecked(settings->snappingEnabled);
    m_sizeSpinBox->setValue(settings->gridSize);
}

QIcon FormsSettingsWidget::icon() const
{
    return QIcon(":/images/settings/forms.png");
}

QString FormsSettingsWidget::title() const
{
    return tr("Forms");
}

bool FormsSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)/*
            || m_fontGroup->title().contains(word, Qt::CaseInsensitive)
            || m_behavioralGroup->title().contains(word, Qt::CaseInsensitive)
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
            || m_preserveDesignerStateCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_preserveDesignerStateCheckBox->toolTip().contains(word, Qt::CaseInsensitive)
            || m_designerStateResetButton->text().contains(word, Qt::CaseInsensitive)
            || m_designerStateResetButton->toolTip().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_themeBox, word)
            || UtilityFunctions::comboContainsWord(m_languageBox, word)
            || UtilityFunctions::comboContainsWord(m_visibleBottomPaneBox, word)*/;
}