#include <scenesettingswidget.h>
#include <scenesettings.h>
#include <designersettings.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <qtcolorbutton.h>

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
  , m_dragStartDistanceLabel(new QLabel(m_designGroup))
  , m_sceneBackgroundColorLabel(new QLabel(m_designGroup))
  , m_sceneZoomLevelLabel(new QLabel(m_designGroup))
  , m_showGuideLinesCheckBox(new QCheckBox(m_designGroup))
  , m_dragStartDistanceSpinBox(new QSpinBox(m_designGroup))
  , m_sceneBackgroundColorBox(new QComboBox(m_designGroup))
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
  , m_controlOutlineLabel(new QLabel(m_controlsGroup))
  , m_outlineColorLabel(new QLabel(m_controlsGroup))
  , m_showMouseoverOutlineCheckBox(new QCheckBox(m_controlsGroup))
  , m_controlOutlineBox(new QComboBox(m_controlsGroup))
  , m_outlineColorButton(new Utils::QtColorButton(m_controlsGroup))
  , m_outlineColorResetButton(new QPushButton(m_controlsGroup))
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
    designLayout->addWidget(m_dragStartDistanceLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneBackgroundColorLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneZoomLevelLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_showGuideLinesCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_dragStartDistanceSpinBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneBackgroundColorBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneZoomLevelBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->setColumnStretch(3, 1);
    designLayout->setColumnMinimumWidth(1, 20);

    m_designGroup->setTitle(tr("Design"));
    m_showGuideLinesLabel->setText(tr("Guide lines") + ":");
    m_dragStartDistanceLabel->setText(tr("Drag start distance") + ":");
    m_sceneBackgroundColorLabel->setText(tr("Background color") + ":");
    m_sceneZoomLevelLabel->setText(tr("Zoom level") + ":");
    m_showGuideLinesCheckBox->setText(tr("Show guide lines"));

    m_showGuideLinesCheckBox->setToolTip(tr("Show guide lines while moving controls"));
    m_dragStartDistanceSpinBox->setToolTip(tr("Change the blocking drag start distace"));
    m_sceneBackgroundColorBox->setToolTip(tr("Change background color of the scene"));
    m_sceneZoomLevelBox->setToolTip(tr("Change zoom level of the scene"));

    m_showGuideLinesCheckBox->setCursor(Qt::PointingHandCursor);
    m_dragStartDistanceSpinBox->setCursor(Qt::PointingHandCursor);
    m_sceneBackgroundColorBox->setCursor(Qt::PointingHandCursor);
    m_sceneZoomLevelBox->setCursor(Qt::PointingHandCursor);

    m_dragStartDistanceSpinBox->setRange(0, 100);
    m_sceneBackgroundColorBox->setIconSize({13, 13});

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
    gridViewLayout->addWidget(m_resetGridViewButton, 2, 2, Qt::AlignRight | Qt::AlignVCenter);
    gridViewLayout->setColumnStretch(3, 1);
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
    controlsLayout->addWidget(m_controlOutlineLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_outlineColorLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_showMouseoverOutlineCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_controlOutlineBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_outlineColorButton, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_outlineColorResetButton, 2, 2, Qt::AlignRight | Qt::AlignVCenter);
    controlsLayout->setColumnStretch(3, 1);
    controlsLayout->setColumnMinimumWidth(1, 20);

    m_controlsGroup->setTitle(tr("Controls") + ":");
    m_showMouseoverOutlineLabel->setText(tr("Mouseover outline") + ":");
    m_controlOutlineLabel->setText(tr("Control outline") + ":");
    m_outlineColorLabel->setText(tr("Outline color") + ":");
    m_showMouseoverOutlineCheckBox->setText(tr("Show mouseover outline"));
    m_outlineColorResetButton->setText(tr("Reset"));

    m_showMouseoverOutlineCheckBox->setToolTip(tr("Show an outline around controls when moving mouse cursor over them"));
    m_controlOutlineBox->setToolTip(tr("Change outline mode for controls"));
    m_outlineColorButton->setToolTip(tr("Change outline color of controls"));
    m_outlineColorResetButton->setToolTip(tr("Reset outline color to default"));

    m_showMouseoverOutlineCheckBox->setCursor(Qt::PointingHandCursor);
    m_controlOutlineBox->setCursor(Qt::PointingHandCursor);
    m_outlineColorButton->setCursor(Qt::PointingHandCursor);
    m_outlineColorResetButton->setCursor(Qt::PointingHandCursor);

    m_controlOutlineBox->setIconSize({14, 14});
    m_outlineColorButton->setFixedWidth(64);

    /****/

    fill();

    connect(m_resetGridViewButton, &QPushButton::clicked, this, [=] {
        const SceneSettings settings(0);
        m_showGridViewDotsCheckBox->setChecked(settings.showGridViewDots);
        m_snappingEnabledCheckBox->setChecked(settings.snappingEnabled);
        m_gridSizeSpinBox->setValue(settings.gridSize);
    });

    connect(m_outlineColorResetButton, &QPushButton::clicked, this, [=] {
        m_outlineColorButton->setColor(SceneSettings(0).outlineColor);
    });

    activate();
    revert();
}

void SceneSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    SceneSettings* settings = DesignerSettings::sceneSettings();
    /****/
    settings->showGuideLines = m_showGuideLinesCheckBox->isChecked();
    settings->dragStartDistance = m_dragStartDistanceSpinBox->value();
    settings->sceneBackgroundColor = m_sceneBackgroundColorBox->currentIndex();
    settings->sceneZoomLevel = UtilityFunctions::textToZoomLevel(m_sceneZoomLevelBox->currentText());
    /****/
    settings->showGridViewDots = m_showGridViewDotsCheckBox->isChecked();
    settings->snappingEnabled = m_snappingEnabledCheckBox->isChecked();
    settings->gridSize = m_gridSizeSpinBox->value();
    /****/
    settings->showMouseoverOutline = m_showMouseoverOutlineCheckBox->isChecked();
    settings->controlOutline = m_controlOutlineBox->currentIndex();
    settings->outlineColor = m_outlineColorButton->color();
    settings->write();
}

void SceneSettingsWidget::revert()
{
    if (!isActivated())
        return;

    activate(false);

    const SceneSettings* settings = DesignerSettings::sceneSettings();
    /****/
    m_showGuideLinesCheckBox->setChecked(settings->showGuideLines);
    m_dragStartDistanceSpinBox->setValue(settings->dragStartDistance);
    m_sceneBackgroundColorBox->setCurrentIndex(settings->sceneBackgroundColor);
    m_sceneZoomLevelBox->setCurrentText(UtilityFunctions::zoomLevelToText(settings->sceneZoomLevel));
    /****/
    m_showGridViewDotsCheckBox->setChecked(settings->showGridViewDots);
    m_snappingEnabledCheckBox->setChecked(settings->snappingEnabled);
    m_gridSizeSpinBox->setValue(settings->gridSize);
    /****/
    m_showMouseoverOutlineCheckBox->setChecked(settings->showMouseoverOutline);
    m_controlOutlineBox->setCurrentIndex(settings->controlOutline);
    m_outlineColorButton->setColor(settings->outlineColor);
}

void SceneSettingsWidget::reset()
{
    DesignerSettings::sceneSettings()->reset();
    DesignerSettings::sceneSettings()->write();
    activate();
    revert();
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
            || m_dragStartDistanceLabel->text().contains(word, Qt::CaseInsensitive)
            || m_sceneBackgroundColorLabel->text().contains(word, Qt::CaseInsensitive)
            || m_sceneZoomLevelLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showGridViewDotsLabel->text().contains(word, Qt::CaseInsensitive)
            || m_snappingEnabledLabel->text().contains(word, Qt::CaseInsensitive)
            || m_gridSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showMouseoverOutlineLabel->text().contains(word, Qt::CaseInsensitive)
            || m_controlOutlineLabel->text().contains(word, Qt::CaseInsensitive)
            || m_outlineColorLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showGuideLinesCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showGridViewDotsCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_snappingEnabledCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showMouseoverOutlineCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_resetGridViewButton->text().contains(word, Qt::CaseInsensitive)
            || m_resetGridViewButton->toolTip().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_sceneBackgroundColorBox, word)
            || UtilityFunctions::comboContainsWord(m_sceneZoomLevelBox, word)
            || UtilityFunctions::comboContainsWord(m_controlOutlineBox, word);
}

void SceneSettingsWidget::fill()
{
    using namespace PaintUtils;
    const QPen pen(Qt::black, 2);
    const qreal dpr = m_sceneBackgroundColorBox->devicePixelRatioF();
    const QSize size = m_sceneBackgroundColorBox->iconSize();

    m_sceneZoomLevelBox->addItems(UtilityFunctions::zoomTexts());

    m_controlOutlineBox->addItem(QIcon(":/images/nooutline.svg"), tr("No outline"));
    m_controlOutlineBox->addItem(QIcon(":/images/outline.svg"), tr("Clipping rect outline"));
    m_controlOutlineBox->addItem(QIcon(":/images/outerline.svg"), tr("Bounding rect outline"));

    m_sceneBackgroundColorBox->addItem(
    {renderPropertyColorPixmap(size, QString(":/images/texture.svg"), pen, dpr)}, tr("Checkered"));
    m_sceneBackgroundColorBox->addItem(
    {renderPropertyColorPixmap(size, Qt::black, pen, dpr)}, tr("Black"));
    m_sceneBackgroundColorBox->addItem(
    {renderPropertyColorPixmap(size, Qt::darkGray, pen, dpr)}, tr("Dark gray"));
    m_sceneBackgroundColorBox->addItem(
    {renderPropertyColorPixmap(size, Qt::lightGray, pen, dpr)}, tr("Light gray"));
    m_sceneBackgroundColorBox->addItem(
    {renderPropertyColorPixmap(size, Qt::white, pen, dpr)},tr("White"));
}
