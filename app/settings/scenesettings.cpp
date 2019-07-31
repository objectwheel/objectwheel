#include <scenesettings.h>
#include <designersettings.h>
#include <QBrush>

static const char g_showGuideLines[] = "ShowGuideLines";
static const char g_dragStartDistance[] = "DragStartDistance";
static const char g_sceneBackgroundTexture[] = "SceneBackgroundTexture";
static const char g_sceneZoomLevel[] = "SceneZoomLevel";
static const char g_showGridViewDots[] = "ShowGridViewDots";
static const char g_snappingEnabled[] = "SnappingEnabled";
static const char g_gridSize[] = "GridSize";
static const char g_showMouseoverOutline[] = "ShowMouseoverOutline";
static const char g_controlOutline[] = "ControlOutline";
static const char g_outlineColor[] = "OutlineColor";

SceneSettings::SceneSettings(DesignerSettings* designerSettings) : Settings(designerSettings)
{
    reset();
}

void SceneSettings::read()
{
    reset();

    begin();
    showGuideLines = value<bool>(g_showGuideLines, showGuideLines);
    dragStartDistance = value<int>(g_dragStartDistance, dragStartDistance);
    sceneBackgroundTexture = value<int>(g_sceneBackgroundTexture, sceneBackgroundTexture);
    sceneZoomLevel = value<qreal>(g_sceneZoomLevel, sceneZoomLevel);
    /****/
    showGridViewDots = value<bool>(g_showGridViewDots, showGridViewDots);
    snappingEnabled = value<bool>(g_snappingEnabled, snappingEnabled);
    gridSize = value<int>(g_gridSize, gridSize);
    /****/
    showMouseoverOutline = value<bool>(g_showMouseoverOutline, showMouseoverOutline);
    controlOutline = value<int>(g_controlOutline, controlOutline);
    outlineColor = value<QColor>(g_outlineColor, outlineColor);
    end();
}

void SceneSettings::write()
{
    begin();
    setValue(g_showGuideLines, showGuideLines);
    setValue(g_dragStartDistance, dragStartDistance);
    setValue(g_sceneBackgroundTexture, sceneBackgroundTexture);
    setValue(g_sceneZoomLevel, sceneZoomLevel);
    /****/
    setValue(g_showGridViewDots, showGridViewDots);
    setValue(g_snappingEnabled, snappingEnabled);
    setValue(g_gridSize, gridSize);
    /****/
    setValue(g_showMouseoverOutline, showMouseoverOutline);
    setValue(g_controlOutline, controlOutline);
    setValue(g_outlineColor, outlineColor);
    end();

    emit static_cast<DesignerSettings*>(groupSettings())->sceneSettingsChanged();
}

void SceneSettings::reset()
{
    showGuideLines = true;
    dragStartDistance = 8;
    sceneBackgroundTexture = 4;
    sceneZoomLevel = 1.0;
    /****/
    showGridViewDots = true;
    snappingEnabled = true;
    gridSize = 8;
    /****/
    showMouseoverOutline = true;
    controlOutline = 0;
    outlineColor = "#2483ec";
}

const char* SceneSettings::category() const
{
    return "Scene";
}

QBrush SceneSettings::toBackgroundBrush() const
{
    QBrush brush(Qt::SolidPattern);
    if (sceneBackgroundTexture == 0)
        brush.setTexture(QPixmap(":/images/texture.svg"));
    else if (sceneBackgroundTexture == 1)
        brush.setColor(Qt::black);
    else if (sceneBackgroundTexture == 2)
        brush.setColor(Qt::darkGray);
    else if (sceneBackgroundTexture == 3)
        brush.setColor(Qt::lightGray);
    else if (sceneBackgroundTexture == 4)
        brush.setColor(Qt::white);
    return brush;
}
