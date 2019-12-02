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
#include <QPainter>

static QPixmap textureImage(const QString& fileName, const QPen& pen, const QSize& size, const QWidget* widget)
{
    QPixmap pixmap(PaintUtils::pixmap(fileName, size, widget));
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRect(QRectF({}, size).adjusted(0.5, 0.5, -0.5, -0.5));
    p.end();
    return pixmap;
}

SceneSettingsWidget::SceneSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_designGroup(new QGroupBox(contentWidget()))
  , m_showGuideLinesLabel(new QLabel(m_designGroup))
  , m_showAllAnchorsLabel(new QLabel(m_designGroup))
  , m_dragStartDistanceLabel(new QLabel(m_designGroup))
  , m_sceneBackgroundTextureLabel(new QLabel(m_designGroup))
  , m_sceneZoomLevelLabel(new QLabel(m_designGroup))
  , m_anchorColorLabel(new QLabel(m_designGroup))
  , m_showGuideLinesCheckBox(new QCheckBox(m_designGroup))
  , m_showAllAnchorsCheckBox(new QCheckBox(m_designGroup))
  , m_dragStartDistanceSpinBox(new QSpinBox(m_designGroup))
  , m_sceneBackgroundTextureBox(new QComboBox(m_designGroup))
  , m_sceneZoomLevelBox(new QComboBox(m_designGroup))
  , m_anchorColorButton(new Utils::QtColorButton(m_designGroup))
  , m_anchorColorResetButton(new QPushButton(m_designGroup))
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
  , m_showClippedControlsLabel(new QLabel(m_controlsGroup))
  , m_blankControlDecorationLabel(new QLabel(m_controlsGroup))
  , m_controlOutlineDecorationLabel(new QLabel(m_controlsGroup))
  , m_controlDoubleClickActionLabel(new QLabel(m_controlsGroup))
  , m_showMouseoverOutlineCheckBox(new QCheckBox(m_controlsGroup))
  , m_showClippedControlsCheckBox(new QCheckBox(m_controlsGroup))
  , m_blankControlDecorationBox(new QComboBox(m_controlsGroup))
  , m_controlOutlineDecorationBox(new QComboBox(m_controlsGroup))
  , m_controlDoubleClickActionBox(new QComboBox(m_controlsGroup))
{
    contentLayout()->addWidget(m_designGroup);
    contentLayout()->addWidget(m_gridViewGroup);
    contentLayout()->addWidget(m_controlsGroup);
    contentLayout()->addStretch();

    /****/

    auto designLayout = new QGridLayout(m_designGroup);
    designLayout->setSpacing(6);
    designLayout->setContentsMargins(4, 4, 4, 4);
    designLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    designLayout->addWidget(m_showGuideLinesLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_showAllAnchorsLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_dragStartDistanceLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneBackgroundTextureLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneZoomLevelLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_anchorColorLabel, 5, 0, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_showGuideLinesCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_showAllAnchorsCheckBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_dragStartDistanceSpinBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneBackgroundTextureBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_sceneZoomLevelBox, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_anchorColorButton, 5, 2, Qt::AlignLeft | Qt::AlignVCenter);
    designLayout->addWidget(m_anchorColorResetButton, 5, 2, Qt::AlignRight | Qt::AlignVCenter);
    designLayout->setColumnStretch(3, 1);
    designLayout->setColumnMinimumWidth(1, 20);

    m_designGroup->setTitle(tr("Design"));
    m_showGuideLinesLabel->setText(tr("Guide lines") + ":");
    m_showAllAnchorsLabel->setText(tr("Anchors") + ":");
    m_dragStartDistanceLabel->setText(tr("Drag start distance") + ":");
    m_sceneBackgroundTextureLabel->setText(tr("Background texture") + ":");
    m_sceneZoomLevelLabel->setText(tr("Zoom level") + ":");
    m_anchorColorLabel->setText(tr("Anchor visualization color") + ":");
    m_showGuideLinesCheckBox->setText(tr("Show guide lines for dragging"));
    m_showAllAnchorsCheckBox->setText(tr("Show/paint all anchors"));
    m_anchorColorResetButton->setText(tr("Reset"));

    m_showGuideLinesCheckBox->setToolTip(tr("Show guide lines while controls are moving"));
    m_showAllAnchorsCheckBox->setToolTip(tr("Show/paint all anchors; if disabled, only "
                                            "anchors for selected controls are going to be visible"));
    m_dragStartDistanceSpinBox->setToolTip(tr("Change the blocking drag start distace"));
    m_sceneBackgroundTextureBox->setToolTip(tr("Change background color of the scene"));
    m_sceneZoomLevelBox->setToolTip(tr("Change zoom level of the scene"));
    m_anchorColorButton->setToolTip(tr("Change anchor visualization color"));
    m_anchorColorResetButton->setToolTip(tr("Reset anchor visualization to default"));

    m_showGuideLinesCheckBox->setCursor(Qt::PointingHandCursor);
    m_showAllAnchorsCheckBox->setCursor(Qt::PointingHandCursor);
    m_dragStartDistanceSpinBox->setCursor(Qt::PointingHandCursor);
    m_sceneBackgroundTextureBox->setCursor(Qt::PointingHandCursor);
    m_sceneZoomLevelBox->setCursor(Qt::PointingHandCursor);
    m_anchorColorButton->setCursor(Qt::PointingHandCursor);
    m_anchorColorResetButton->setCursor(Qt::PointingHandCursor);

    m_dragStartDistanceSpinBox->setRange(0, 100);
    m_sceneBackgroundTextureBox->setIconSize({13, 13});
    m_anchorColorButton->setFixedWidth(64);

    /****/

    auto gridViewLayout = new QGridLayout(m_gridViewGroup);
    gridViewLayout->setSpacing(6);
    gridViewLayout->setContentsMargins(4, 4, 4, 4);
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
    controlsLayout->setSpacing(6);
    controlsLayout->setContentsMargins(4, 4, 4, 4);
    controlsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    controlsLayout->addWidget(m_showMouseoverOutlineLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_showClippedControlsLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_blankControlDecorationLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_controlOutlineDecorationLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_controlDoubleClickActionLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_showMouseoverOutlineCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_showClippedControlsCheckBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_blankControlDecorationBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_controlOutlineDecorationBox, 3, 2, Qt::AlignLeft | Qt::AlignVCenter);
    controlsLayout->addWidget(m_controlDoubleClickActionBox, 4, 2, Qt::AlignLeft | Qt::AlignVCenter);

    controlsLayout->setColumnStretch(3, 1);
    controlsLayout->setColumnMinimumWidth(1, 20);

    m_controlsGroup->setTitle(tr("Controls") + ":");
    m_showMouseoverOutlineLabel->setText(tr("Mouseover outline") + ":");
    m_showClippedControlsLabel->setText(tr("Control clipping") + ":");
    m_blankControlDecorationLabel->setText(tr("Blank control decoration") + ":");
    m_controlOutlineDecorationLabel->setText(tr("Control outline decoration") + ":");
    m_controlDoubleClickActionLabel->setText(tr("Control double click action") + ":");
    m_showMouseoverOutlineCheckBox->setText(tr("Show mouseover outline"));
    m_showClippedControlsCheckBox->setText(tr("Show clipped controls"));

    m_showMouseoverOutlineCheckBox->setToolTip(tr("Show an outline around controls when moving mouse cursor over them"));
    m_showClippedControlsCheckBox->setToolTip(tr("Show controls even if they are clipped out by their parent control"));
    m_blankControlDecorationBox->setToolTip(tr("Change decoration for blank controls with transparent content"));
    m_controlOutlineDecorationBox->setToolTip(tr("Change decoration for control outlines"));
    m_controlDoubleClickActionBox->setToolTip(tr("Change default action for the double clicks on controls"));

    m_showMouseoverOutlineCheckBox->setCursor(Qt::PointingHandCursor);
    m_showClippedControlsCheckBox->setCursor(Qt::PointingHandCursor);
    m_blankControlDecorationBox->setCursor(Qt::PointingHandCursor);
    m_controlOutlineDecorationBox->setCursor(Qt::PointingHandCursor);
    m_controlDoubleClickActionBox->setCursor(Qt::PointingHandCursor);

    /****/

    fill();

    connect(m_resetGridViewButton, &QPushButton::clicked, this, [=] {
        const SceneSettings settings(0);
        m_showGridViewDotsCheckBox->setChecked(settings.showGridViewDots);
        m_snappingEnabledCheckBox->setChecked(settings.snappingEnabled);
        m_gridSizeSpinBox->setValue(settings.gridSize);
    });

    connect(m_anchorColorResetButton, &QPushButton::clicked, this, [=] {
        m_anchorColorButton->setColor(SceneSettings(0).anchorColor);
    });

    connect(DesignerSettings::instance(), &DesignerSettings::sceneSettingsChanged, this, [=] {
        activate();
        revert();
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
    settings->showAllAnchors = m_showAllAnchorsCheckBox->isChecked();
    settings->dragStartDistance = m_dragStartDistanceSpinBox->value();
    settings->sceneBackgroundTexture = m_sceneBackgroundTextureBox->currentIndex();
    settings->sceneZoomLevel = UtilityFunctions::textToZoomLevel(m_sceneZoomLevelBox->currentText());
    settings->anchorColor = m_anchorColorButton->color();
    /****/
    settings->showGridViewDots = m_showGridViewDotsCheckBox->isChecked();
    settings->snappingEnabled = m_snappingEnabledCheckBox->isChecked();
    settings->gridSize = m_gridSizeSpinBox->value();
    /****/
    settings->showMouseoverOutline = m_showMouseoverOutlineCheckBox->isChecked();
    settings->showClippedControls = m_showClippedControlsCheckBox->isChecked();
    settings->blankControlDecoration = m_blankControlDecorationBox->currentIndex();
    settings->controlOutlineDecoration = m_controlOutlineDecorationBox->currentIndex();
    settings->controlDoubleClickAction = m_controlDoubleClickActionBox->currentIndex();
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
    m_showAllAnchorsCheckBox->setChecked(settings->showAllAnchors);
    m_dragStartDistanceSpinBox->setValue(settings->dragStartDistance);
    m_sceneBackgroundTextureBox->setCurrentIndex(settings->sceneBackgroundTexture);
    m_sceneZoomLevelBox->setCurrentText(UtilityFunctions::zoomLevelToText(settings->sceneZoomLevel));
    m_anchorColorButton->setColor(settings->anchorColor);
    /****/
    m_showGridViewDotsCheckBox->setChecked(settings->showGridViewDots);
    m_snappingEnabledCheckBox->setChecked(settings->snappingEnabled);
    m_gridSizeSpinBox->setValue(settings->gridSize);
    /****/
    m_showMouseoverOutlineCheckBox->setChecked(settings->showMouseoverOutline);
    m_showClippedControlsCheckBox->setChecked(settings->showClippedControls);
    m_blankControlDecorationBox->setCurrentIndex(settings->blankControlDecoration);
    m_controlOutlineDecorationBox->setCurrentIndex(settings->controlOutlineDecoration);
    m_controlDoubleClickActionBox->setCurrentIndex(settings->controlDoubleClickAction);
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
    return QIcon(":/images/settings/scene.svg");
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
            || m_showAllAnchorsLabel->text().contains(word, Qt::CaseInsensitive)
            || m_dragStartDistanceLabel->text().contains(word, Qt::CaseInsensitive)
            || m_sceneBackgroundTextureLabel->text().contains(word, Qt::CaseInsensitive)
            || m_sceneZoomLevelLabel->text().contains(word, Qt::CaseInsensitive)
            || m_anchorColorLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showGridViewDotsLabel->text().contains(word, Qt::CaseInsensitive)
            || m_snappingEnabledLabel->text().contains(word, Qt::CaseInsensitive)
            || m_gridSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showMouseoverOutlineLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showClippedControlsLabel->text().contains(word, Qt::CaseInsensitive)
            || m_blankControlDecorationLabel->text().contains(word, Qt::CaseInsensitive)
            || m_controlOutlineDecorationLabel->text().contains(word, Qt::CaseInsensitive)
            || m_controlDoubleClickActionLabel->text().contains(word, Qt::CaseInsensitive)
            || m_showGuideLinesCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showAllAnchorsCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showGridViewDotsCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_snappingEnabledCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showMouseoverOutlineCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_showClippedControlsCheckBox->text().contains(word, Qt::CaseInsensitive)
            || m_resetGridViewButton->text().contains(word, Qt::CaseInsensitive)
            || m_resetGridViewButton->toolTip().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_sceneBackgroundTextureBox, word)
            || UtilityFunctions::comboContainsWord(m_sceneZoomLevelBox, word)
            || UtilityFunctions::comboContainsWord(m_controlOutlineDecorationBox, word)
            || UtilityFunctions::comboContainsWord(m_controlDoubleClickActionBox, word);
}

void SceneSettingsWidget::fill()
{
    using namespace PaintUtils;
    const QPen pen(Qt::black, 2);
    const qreal dpr = m_sceneBackgroundTextureBox->devicePixelRatioF();
    const QSize size = m_sceneBackgroundTextureBox->iconSize();

    m_sceneZoomLevelBox->addItems(UtilityFunctions::zoomTexts());

    m_controlOutlineDecorationBox->addItem(tr("No Outline"));
    m_controlOutlineDecorationBox->addItem(tr("Clipping Dash Line"));
    m_controlOutlineDecorationBox->addItem(tr("Bounding Dash Line"));
    m_controlOutlineDecorationBox->addItem(tr("Clipping Solid Line"));
    m_controlOutlineDecorationBox->addItem(tr("Bounding Solid Line"));

    m_blankControlDecorationBox->addItem(tr("No Decoration"));
    m_blankControlDecorationBox->addItem(tr("Id Only"));
    m_blankControlDecorationBox->addItem(tr("Id + Diag Pattern"));
    m_blankControlDecorationBox->addItem(tr("Id + Dense Pattern"));

    m_controlDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/edit-anchors.svg")),
                                           tr("Edit Anchors Action"));
    m_controlDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/view-source-code.svg")),
                                           tr("View Source Code Action"));
    m_controlDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/go-to-slot.svg")),
                                           tr("Go to Slot Action"));

    m_sceneBackgroundTextureBox->addItem(
    {textureImage(":/images/settings/texture.svg", pen, size, this)}, tr("Checkered"));
    m_sceneBackgroundTextureBox->addItem(
    {renderPropertyColorPixmap(size, Qt::black, pen, dpr)}, tr("Black"));
    m_sceneBackgroundTextureBox->addItem(
    {renderPropertyColorPixmap(size, Qt::darkGray, pen, dpr)}, tr("Dark Gray"));
    m_sceneBackgroundTextureBox->addItem(
    {renderPropertyColorPixmap(size, Qt::lightGray, pen, dpr)}, tr("Light Gray"));
    m_sceneBackgroundTextureBox->addItem(
    {renderPropertyColorPixmap(size, Qt::white, pen, dpr)},tr("White"));
}
