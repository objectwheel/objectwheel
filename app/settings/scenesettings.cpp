#include <scenesettings.h>
#include <designersettings.h>

static const char g_showGuideLines[] = "ShowGuideLines";
static const char g_sceneBackgroundColorMode[] = "SceneBackgroundColorMode";
static const char g_sceneZoomLevel[] = "SceneZoomLevel";
static const char g_showGridViewDots[] = "ShowGridViewDots";
static const char g_snappingEnabled[] = "SnappingEnabled";
static const char g_gridSize[] = "GridSize";
static const char g_showMouseoverOutline[] = "ShowMouseoverOutline";
static const char g_controlOutlineMode[] = "ControlOutlineMode";

SceneSettings::SceneSettings(DesignerSettings* designerSettings) : Settings(designerSettings)
{
    reset();
}

void SceneSettings::read()
{
    reset();

    begin();
    showGuideLines = value<bool>(g_showGuideLines, showGuideLines);
    sceneBackgroundColorMode = value<int>(g_sceneBackgroundColorMode, sceneBackgroundColorMode);
    sceneZoomLevel = value<qreal>(g_sceneZoomLevel, sceneZoomLevel);
    /****/
    showGridViewDots = value<bool>(g_showGridViewDots, showGridViewDots);
    snappingEnabled = value<bool>(g_snappingEnabled, snappingEnabled);
    gridSize = value<int>(g_gridSize, gridSize);
    /****/
    showMouseoverOutline = value<bool>(g_showMouseoverOutline, showMouseoverOutline);
    controlOutlineMode = value<int>(g_controlOutlineMode, controlOutlineMode);
    end();
}

void SceneSettings::write()
{
    begin();
    setValue(g_showGuideLines, showGuideLines);
    setValue(g_sceneBackgroundColorMode, sceneBackgroundColorMode);
    setValue(g_sceneZoomLevel, sceneZoomLevel);
    /****/
    setValue(g_showGridViewDots, showGridViewDots);
    setValue(g_snappingEnabled, snappingEnabled);
    setValue(g_gridSize, gridSize);
    /****/
    setValue(g_showMouseoverOutline, showMouseoverOutline);
    setValue(g_controlOutlineMode, controlOutlineMode);
    end();

    emit static_cast<DesignerSettings*>(groupSettings())->sceneSettingsChanged();
}

void SceneSettings::reset()
{
    showGuideLines = true;
    sceneBackgroundColorMode = 0;
    sceneZoomLevel = 1.0;
    /****/
    showGridViewDots = true;
    snappingEnabled = true;
    gridSize = 8;
    /****/
    showMouseoverOutline = true;
    controlOutlineMode = 0;
}

const char* SceneSettings::category() const
{
    return "Scene";
}

