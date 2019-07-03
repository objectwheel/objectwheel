#include <scenesettingswidget.h>
#include <scenesettings.h>
#include <designersettings.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QSpinBox>
#include <QPen>

SceneSettingsWidget::SceneSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_designGroup(new QGroupBox(contentWidget()))
  , m_showGuideLinesLabel(new QLabel(m_designGroup))
  , m_sceneBackgroundColorModeLabel(new QLabel(m_designGroup))
  , m_sceneZoomLevelLabel(new QLabel(m_designGroup))
  , m_showGuideLinesCheckBox(new QCheckBox(m_designGroup))
  , m_sceneBackgroundColorModeBox(new QComboBox(m_designGroup))
  , m_sceneZoomLevelBox(new QComboBox(m_designGroup))
  /****/
  , m_gridViewGroup(new QGroupBox(contentWidget()))
  , m_showGridViewDotsLabel(new QLabel(m_gridViewGroup))
  , m_snappingEnabledLabel(new QLabel(m_gridViewGroup))
  , m_gridSizeLabel(new QLabel(m_gridViewGroup))
  , m_showGridViewDotsCheckBox(new QCheckBox(m_gridViewGroup))
  , m_snappingEnabledCheckBox(new QCheckBox(m_gridViewGroup))
  , m_gridSizeSpinBox(new QSpinBox(m_gridViewGroup))
  , m_resetGridViewButton(new QPushButton(m_gridViewGroup))
  /****/
  , m_controlsGroup(new QGroupBox(contentWidget()))
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

    auto designLayout = new QGridLayout(m_designGroup);
    designLayout->setSpacing(8);
    designLayout->setContentsMargins(6, 6, 6, 6);
    designLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    designLayout->addWidget(m_showGuideLinesLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneBackgroundColorModeLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneZoomLevelLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_showGuideLinesCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneBackgroundColorModeBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneZoomLevelBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->setColumnStretch(3, 1);
    designLayout->setColumnMinimumWidth(1, 20);

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

    m_sceneBackgroundColorModeBox->setIconSize({13, 13});

    /****/

    auto gridViewLayout = new QGridLayout(m_gridViewGroup);
    gridViewLayout->setSpacing(8);
    gridViewLayout->setContentsMargins(6, 6, 6, 6);
    gridViewLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    gridViewLayout->addWidget(m_showGridViewDotsLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    gridViewLayout->addWidget(m_snappingEnabledLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    gridViewLayout->addWidget(m_gridSizeLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    gridViewLayout->addWidget(m_showGridViewDotsCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    gridViewLayout->addWidget(m_snappingEnabledCheckBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    gridViewLayout->addWidget(m_gridSizeSpinBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    gridViewLayout->addWidget(m_resetGridViewButton, 2, 3, Qt::AlignLeft | Qt::AlignVCenter);
    gridViewLayout->setColumnStretch(4, 1);
    gridViewLayout->setColumnMinimumWidth(1, 20);

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

    auto controlsLayout = new QGridLayout(m_controlsGroup);
    controlsLayout->setSpacing(8);
    controlsLayout->setContentsMargins(6, 6, 6, 6);
    controlsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    controlsLayout->addWidget(m_showMouseoverOutlineLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_controlOutlineModeLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_showMouseoverOutlineCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_controlOutlineModeBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->setColumnStretch(3, 1);
    controlsLayout->setColumnMinimumWidth(1, 20);

    m_controlsGroup->setTitle(tr("Controls") + ":");
    m_showMouseoverOutlineLabel->setText(tr("Mouseover outline") + ":");
    m_controlOutlineModeLabel->setText(tr("Control outline") + ":");
    m_showMouseoverOutlineCheckBox->setText(tr("Show mouseover outline"));

    m_showMouseoverOutlineCheckBox->setToolTip(tr("Show an outline around controls when moving mouse cursor over them"));
    m_controlOutlineModeBox->setToolTip(tr("Change default outline mode for controls"));

    m_showMouseoverOutlineCheckBox->setCursor(Qt::PointingHandCursor);
    m_controlOutlineModeBox->setCursor(Qt::PointingHandCursor);

    m_controlOutlineModeBox->setIconSize({14, 14});

    /****/

    fill();

    connect(m_resetGridViewButton, &QPushButton::clicked, this, [=] {
        const SceneSettings settings(0);
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
    settings->sceneZoomLevel = UtilityFunctions::textToZoomLevel(m_sceneZoomLevelBox->currentText());
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
    m_sceneZoomLevelBox->setCurrentText(UtilityFunctions::zoomLevelToText(settings->sceneZoomLevel));
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
    return title().contains(word, Qt::CaseInsensitive)
            || m_designGroup->title().contains(word, Qt::CaseInsensitive)
            || m_gridViewGroup->title().contains(word, Qt::CaseInsensitive)
            || m_controlsGroup->title().contains(word, Qt::CaseInsensitive)
            || m_showGuideLinesLabel->text().contains(word, Qt::CaseInsensitive)
            || m_sceneBackgroundColorModeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_sceneZoomLevelLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showGridViewDotsLabel->text().contains(word, Qt::CaseInsensitive)
            || m_snappingEnabledLabel->text().contains(word, Qt::CaseInsensitive)
            || m_gridSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showMouseoverOutlineLabel->text().contains(word, Qt::CaseInsensitive)
            || m_controlOutlineModeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showGuideLinesCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showGridViewDotsCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_snappingEnabledCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showMouseoverOutlineCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_resetGridViewButton->text().contains(word, Qt::CaseInsensitive)
            || m_resetGridViewButton->toolTip().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_sceneBackgroundColorModeBox, word)
            || UtilityFunctions::comboContainsWord(m_sceneZoomLevelBox, word)
            || UtilityFunctions::comboContainsWord(m_controlOutlineModeBox, word);
}

void SceneSettingsWidget::fill()
{
    using namespace PaintUtils;
    const QPen pen(Qt::black, 2);
    const qreal dpr = m_sceneBackgroundColorModeBox->devicePixelRatioF();
    const QSize size = m_sceneBackgroundColorModeBox->iconSize();

    m_sceneZoomLevelBox->addItems(UtilityFunctions::zoomTexts());

    m_controlOutlineModeBox->addItem(QIcon(":/images/nooutline.svg"), tr("No outline"));
    m_controlOutlineModeBox->addItem(QIcon(":/images/outline.svg"), tr("Clipping rect outline"));
    m_controlOutlineModeBox->addItem(QIcon(":/images/outerline.svg"), tr("Bounding rect outline"));

    m_sceneBackgroundColorModeBox->addItem(
    {renderPropertyColorPixmap(size, QString(":/images/texture.svg"), pen, dpr)}, tr("Checkered"));
    m_sceneBackgroundColorModeBox->addItem(
    {renderPropertyColorPixmap(size, Qt::black, pen, dpr)}, tr("Black"));
    m_sceneBackgroundColorModeBox->addItem(
    {renderPropertyColorPixmap(size, Qt::darkGray, pen, dpr)}, tr("Dark gray"));
    m_sceneBackgroundColorModeBox->addItem(
    {renderPropertyColorPixmap(size, Qt::lightGray, pen, dpr)}, tr("Light gray"));
    m_sceneBackgroundColorModeBox->addItem(
    {renderPropertyColorPixmap(size, Qt::white, pen, dpr)},tr("White"));
}
