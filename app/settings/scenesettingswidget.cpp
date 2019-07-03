#include <scenesettingswidget.h>
#include <scenesettings.h>
#include <designersettings.h>
#include <paintutils.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QSpinBox>
#include <QPen>

static const char* g_zoomLevels[] = {
    "10 %", "25 %", "50 %", "75 %", "90 %", "100 %", "125 %",
    "150 %", "175 %", "200 %", "300 %", "500 %", "1000 %"
};
static const qreal g_zoomRatios[] = {
    0.1, 0.25, 0.5, 0.75, 0.9, 1.0, 1.25,
    1.5, 1.75, 2.0, 3.0, 5.0, 10.0
};

const char* toText(qreal ratio)
{
    for (size_t i = 0; i < sizeof(g_zoomRatios) / sizeof(g_zoomRatios[0]); ++i) {
        if (g_zoomRatios[i] == ratio)
            return g_zoomLevels[i];
    }
    return "100 %";
}

qreal toRatio(const QString& text)
{
    for (size_t i = 0; i < sizeof(g_zoomLevels) / sizeof(g_zoomLevels[0]); ++i) {
        if (g_zoomLevels[i] == text)
            return g_zoomRatios[i];
    }
    return 1.0;
}

void addZoomLevels(QComboBox* comboBox)
{
    for (size_t i = 0; i < sizeof(g_zoomLevels) / sizeof(g_zoomLevels[0]); ++i)
        comboBox->addItem(QObject::tr(g_zoomLevels[i]), g_zoomLevels[i]);
}

void addOutlines(QComboBox* comboBox)
{
    comboBox->addItem(QIcon(":/images/nooutline.svg"), QObject::tr("No outline"));
    comboBox->addItem(QIcon(":/images/outline.svg"), QObject::tr("Clipping rect outline"));
    comboBox->addItem(QIcon(":/images/outerline.svg"), QObject::tr("Bounding rect outline"));
}

void addBackgroundColors(QComboBox* comboBox)
{
    using namespace PaintUtils;
    const qreal dpr = comboBox->devicePixelRatioF();
    const QSize size = comboBox->iconSize();
    QPen pen;
    pen.setWidth(2);
    comboBox->addItem(
                QIcon(renderPropertyColorPixmap(size, QString(":/images/texture.svg"), pen, dpr)),
                QObject::tr("Checkered"));
    comboBox->addItem(
                QIcon(renderPropertyColorPixmap(size, Qt::black, pen, dpr)),
                QObject::tr("Black"));
    comboBox->addItem(
                QIcon(renderPropertyColorPixmap(size, Qt::darkGray, pen, dpr)),
                QObject::tr("Dark gray"));
    comboBox->addItem(
                QIcon(renderPropertyColorPixmap(size, Qt::lightGray, pen, dpr)),
                QObject::tr("Light gray"));
    comboBox->addItem(
                QIcon(renderPropertyColorPixmap(size, Qt::white, pen, dpr)),
                QObject::tr("White"));
}

SceneSettingsWidget::SceneSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_designGroup(new QGroupBox(contentWidget()))
  , m_designLayout(new QGridLayout(m_designGroup))
  , m_showGuideLinesLabel(new QLabel(m_designGroup))
  , m_sceneBackgroundColorModeLabel(new QLabel(m_designGroup))
  , m_sceneZoomLevelLabel(new QLabel(m_designGroup))
  , m_showGuideLinesCheckBox(new QCheckBox(m_designGroup))
  , m_sceneBackgroundColorModeBox(new QComboBox(m_designGroup))
  , m_sceneZoomLevelBox(new QComboBox(m_designGroup))
  /****/
  , m_gridViewGroup(new QGroupBox(contentWidget()))
  , m_gridViewLayout(new QGridLayout(m_gridViewGroup))
  , m_showGridViewDotsLabel(new QLabel(m_gridViewGroup))
  , m_snappingEnabledLabel(new QLabel(m_gridViewGroup))
  , m_gridSizeLabel(new QLabel(m_gridViewGroup))
  , m_showGridViewDotsCheckBox(new QCheckBox(m_gridViewGroup))
  , m_snappingEnabledCheckBox(new QCheckBox(m_gridViewGroup))
  , m_gridSizeSpinBox(new QSpinBox(m_gridViewGroup))
  , m_resetGridViewButton(new QPushButton(m_gridViewGroup))
  /****/
  , m_controlsGroup(new QGroupBox(contentWidget()))
  , m_controlsLayout(new QGridLayout(m_controlsGroup))
  , m_showMouseoverOutlineLabel(new QLabel(m_controlsGroup))
  , m_controlOutlineModeLabel(new QLabel(m_controlsGroup))
  , m_showMouseoverOutlineCheckBox(new QCheckBox(m_controlsGroup))
  , m_controlOutlineModeBox(new QComboBox(m_controlsGroup))
{
    contentLayout()->addWidget(m_designGroup);
    contentLayout()->addWidget(m_gridViewGroup);
    contentLayout()->addWidget(m_controlsGroup);
    contentLayout()->addStretch();

    /****/

    m_designLayout->setSpacing(8);
    m_designLayout->setContentsMargins(6, 6, 6, 6);
    m_designLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_designLayout->addWidget(m_showGuideLinesLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_sceneBackgroundColorModeLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_sceneZoomLevelLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_showGuideLinesCheckBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_sceneBackgroundColorModeBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->addWidget(m_sceneZoomLevelBox, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_designLayout->setColumnStretch(3, 1);
    m_designLayout->setColumnMinimumWidth(1, 20);

    m_designGroup->setTitle(tr("Design"));
    m_showGuideLinesLabel->setText(tr("Guide lines") + ":");
    m_sceneBackgroundColorModeLabel->setText(tr("Background color") + ":");
    m_sceneZoomLevelLabel->setText(tr("Zoom level") + ":");
    m_showGuideLinesCheckBox->setText(tr("Show guide lines"));

    m_showGuideLinesCheckBox->setToolTip(tr("Show guide lines while moving controls"));
    m_sceneBackgroundColorModeBox->setToolTip(tr("Change background color of the scene"));
    m_sceneZoomLevelBox->setToolTip(tr("Change zoom level of the scene"));

    m_showGuideLinesCheckBox->setCursor(Qt::PointingHandCursor);
    m_sceneBackgroundColorModeBox->setCursor(Qt::PointingHandCursor);
    m_sceneZoomLevelBox->setCursor(Qt::PointingHandCursor);

    m_sceneBackgroundColorModeBox->setIconSize({14, 14});

    /****/

    m_gridViewLayout->setSpacing(8);
    m_gridViewLayout->setContentsMargins(6, 6, 6, 6);
    m_gridViewLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_gridViewLayout->addWidget(m_showGridViewDotsLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridViewLayout->addWidget(m_snappingEnabledLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridViewLayout->addWidget(m_gridSizeLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridViewLayout->addWidget(m_showGridViewDotsCheckBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridViewLayout->addWidget(m_snappingEnabledCheckBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridViewLayout->addWidget(m_gridSizeSpinBox, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridViewLayout->addWidget(m_resetGridViewButton, 4, 3, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridViewLayout->setColumnStretch(4, 1);
    m_gridViewLayout->setColumnMinimumWidth(1, 20);

    m_gridViewGroup->setTitle(tr("Grid View") + ":");
    m_showGridViewDotsLabel->setText(tr("Grid view dots") + ":");
    m_snappingEnabledLabel->setText(tr("Snapping") + ":");
    m_gridSizeLabel->setText(tr("Grid size") + ":");
    m_showGridViewDotsCheckBox->setText(tr("Show grid view dots"));
    m_snappingEnabledCheckBox->setText(tr("Enable snapping"));
    m_resetGridViewButton->setText(tr("Reset"));

    m_showGridViewDotsCheckBox->setToolTip(tr("Show grid view dots"));
    m_snappingEnabledCheckBox->setToolTip(tr("Enable grid view snapping"));
    m_gridSizeSpinBox->setToolTip(tr("Change grid size"));
    m_resetGridViewButton->setToolTip(tr("Reset grid view settings to default"));

    m_showGridViewDotsCheckBox->setCursor(Qt::PointingHandCursor);
    m_snappingEnabledCheckBox->setCursor(Qt::PointingHandCursor);
    m_gridSizeSpinBox->setCursor(Qt::PointingHandCursor);
    m_resetGridViewButton->setCursor(Qt::PointingHandCursor);

    m_gridSizeSpinBox->setMinimum(1);
    m_gridSizeSpinBox->setMaximum(99);
    /****/

    m_controlsLayout->setSpacing(8);
    m_controlsLayout->setContentsMargins(6, 6, 6, 6);
    m_controlsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_controlsLayout->addWidget(m_showMouseoverOutlineLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_controlsLayout->addWidget(m_controlOutlineModeLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_controlsLayout->addWidget(m_showMouseoverOutlineCheckBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_controlsLayout->addWidget(m_controlOutlineModeBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_controlsLayout->setColumnStretch(3, 1);
    m_controlsLayout->setColumnMinimumWidth(1, 20);

    m_controlsGroup->setTitle(tr("Controls") + ":");
    m_showMouseoverOutlineLabel->setText(tr("Mouseover outline") + ":");
    m_controlOutlineModeLabel->setText(tr("Control outline") + ":");
    m_showMouseoverOutlineCheckBox->setText(tr("Show mouseover outline"));

    m_showMouseoverOutlineCheckBox->setToolTip(tr("Show an outline around controls when moving mouse cursor over them"));
    m_controlOutlineModeBox->setToolTip(tr("Change default outline mode for controls"));

    m_showMouseoverOutlineCheckBox->setCursor(Qt::PointingHandCursor);
    m_controlOutlineModeBox->setCursor(Qt::PointingHandCursor);

    /****/

    addZoomLevels(m_sceneZoomLevelBox);
    addOutlines(m_controlOutlineModeBox);
    addBackgroundColors(m_sceneBackgroundColorModeBox);

    connect(m_resetGridViewButton, &QPushButton::clicked, this, [=] {
        const SceneSettings settings;
        m_showGridViewDotsCheckBox->setChecked(settings.showGridViewDots);
        m_snappingEnabledCheckBox->setChecked(settings.snappingEnabled);
        m_gridSizeSpinBox->setValue(settings.gridSize);
    });

    activate();
    reset();
}

void SceneSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    SceneSettings* settings = DesignerSettings::sceneSettings();
    /****/
    settings->showGuideLines = m_showGuideLinesCheckBox->isChecked();
    settings->sceneBackgroundColorMode = m_sceneBackgroundColorModeBox->currentIndex();
    settings->sceneZoomLevel = toRatio(m_sceneZoomLevelBox->currentText());
    /****/
    settings->showGridViewDots = m_showGridViewDotsCheckBox->isChecked();
    settings->snappingEnabled = m_snappingEnabledCheckBox->isChecked();
    settings->gridSize = m_gridSizeSpinBox->value();
    /****/
    settings->showMouseoverOutline = m_showMouseoverOutlineCheckBox->isChecked();
    settings->controlOutlineMode = m_controlOutlineModeBox->currentIndex();
    settings->write();
}

void SceneSettingsWidget::reset()
{
    if (!isActivated())
        return;

    activate(false);

    const SceneSettings* settings = DesignerSettings::sceneSettings();
    /****/
    m_showGuideLinesCheckBox->setChecked(settings->showGuideLines);
    m_sceneBackgroundColorModeBox->setCurrentIndex(settings->sceneBackgroundColorMode);
    m_sceneZoomLevelBox->setCurrentText(toText(settings->sceneZoomLevel));
    /****/
    m_showGridViewDotsCheckBox->setChecked(settings->showGridViewDots);
    m_snappingEnabledCheckBox->setChecked(settings->snappingEnabled);
    m_gridSizeSpinBox->setValue(settings->gridSize);
    /****/
    m_showMouseoverOutlineCheckBox->setChecked(settings->showMouseoverOutline);
    m_controlOutlineModeBox->setCurrentIndex(settings->controlOutlineMode);
}

QIcon SceneSettingsWidget::icon() const
{
    return QIcon(":/images/settings/scene.png");
}

QString SceneSettingsWidget::title() const
{
    return tr("Scene");
}

bool SceneSettingsWidget::containsWord(const QString& word) const
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