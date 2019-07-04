#include <scenesettings.h>
#include <designersettings.h>
#include <QBrush>

static const char g_showGuideLines[] = "ShowGuideLines";
static const char g_sceneBackgroundColor[] = "SceneBackgroundColor";
static const char g_sceneZoomLevel[] = "SceneZoomLevel";
static const char g_showGridViewDots[] = "ShowGridViewDots";
static const char g_snappingEnabled[] = "SnappingEnabled";
static const char g_gridSize[] = "GridSize";
static const char g_showMouseoverOutline[] = "ShowMouseoverOutline";
static const char g_controlOutline[] = "ControlOutline";

SceneSettings::SceneSettings(DesignerSettings* designerSettings) : Settings(designerSettings)
{
    reset();
}

void SceneSettings::read()
{
    reset();

    begin();
    showGuideLines = value<bool>(g_showGuideLines, showGuideLines);
    sceneBackgroundColor = value<int>(g_sceneBackgroundColor, sceneBackgroundColor);
    sceneZoomLevel = value<qreal>(g_sceneZoomLevel, sceneZoomLevel);
    /****/
    showGridViewDots = value<bool>(g_showGridViewDots, showGridViewDots);
    snappingEnabled = value<bool>(g_snappingEnabled, snappingEnabled);
    gridSize = value<int>(g_gridSize, gridSize);
    /****/
    showMouseoverOutline = value<bool>(g_showMouseoverOutline, showMouseoverOutline);
    controlOutline = value<int>(g_controlOutline, controlOutline);
    end();
}

void SceneSettings::write()
{
    begin();
    setValue(g_showGuideLines, showGuideLines);
    setValue(g_sceneBackgroundColor, sceneBackgroundColor);
    setValue(g_sceneZoomLevel, sceneZoomLevel);
    /****/
    setValue(g_showGridViewDots, showGridViewDots);
    setValue(g_snappingEnabled, snappingEnabled);
    setValue(g_gridSize, gridSize);
    /****/
    setValue(g_showMouseoverOutline, showMouseoverOutline);
    setValue(g_controlOutline, controlOutline);
    end();

    emit static_cast<DesignerSettings*>(groupSettings())->sceneSettingsChanged();
}

void SceneSettings::reset()
{
    showGuideLines = true;
    sceneBackgroundColor = 0;
    sceneZoomLevel = 1.0;
    /****/
    showGridViewDots = true;
    snappingEnabled = true;
    gridSize = 8;
    /****/
    showMouseoverOutline = true;
    controlOutline = 0;
}

const char* SceneSettings::category() const
{
    return "Scene";
}

QBrush SceneSettings::toBackgroundBrush() const
{
    QBrush brush(Qt::SolidPattern);
    if (sceneBackgroundColor == 0)
        brush.setTexture(QPixmap(":/images/texture.svg"));
    else if (sceneBackgroundColor == 1)
        brush.setColor(Qt::black);
    else if (sceneBackgroundColor == 2)
        brush.setColor(Qt::darkGray);
    else if (sceneBackgroundColor == 3)
        brush.setColor(Qt::lightGray);
    else if (sceneBackgroundColor == 4)
        brush.setColor(Qt::white);
    return brush;
}
