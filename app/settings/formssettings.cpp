#include <formssettings.h>
#include <designersettings.h>

static const char g_guidelinesVisible[] = "GuidelinesVisible";
static const char g_hoverOutlineVisible[] = "HoverOutlineVisible";
static const char g_backgroundColorMode[] = "BackgroundColorMode";
static const char g_outlineMode[] = "OutlineMode";
static const char g_zoomLevel[] = "ZoomLevel";
static const char g_gridViewVisible[] = "GridViewVisible";
static const char g_snappingEnabled[] = "SnappingEnabled";
static const char g_gridSize[] = "GridSize";

FormsSettings::FormsSettings(DesignerSettings* designerSettings) : Settings(designerSettings)
{
    reset();
}

void FormsSettings::read()
{
    reset();

    begin();
    guidelinesVisible = value<bool>(g_guidelinesVisible, guidelinesVisible);
    hoverOutlineVisible = value<bool>(g_hoverOutlineVisible, hoverOutlineVisible);
    backgroundColorMode = value<int>(g_backgroundColorMode, backgroundColorMode);
    outlineMode = value<int>(g_outlineMode, outlineMode);
    zoomLevel = value<qreal>(g_zoomLevel, zoomLevel);    
    /****/
    gridViewVisible = value<bool>(g_gridViewVisible, gridViewVisible);
    snappingEnabled = value<bool>(g_snappingEnabled, snappingEnabled);
    gridSize = value<int>(g_gridSize, gridSize);
    end();
}

void FormsSettings::write()
{
    begin();
    setValue(g_guidelinesVisible, guidelinesVisible);
    setValue(g_hoverOutlineVisible, hoverOutlineVisible);
    setValue(g_backgroundColorMode, backgroundColorMode);
    setValue(g_outlineMode, outlineMode);
    setValue(g_zoomLevel, zoomLevel);
    /****/
    setValue(g_gridViewVisible, gridViewVisible);
    setValue(g_snappingEnabled, snappingEnabled);
    setValue(g_gridSize, gridSize);
    end();

    emit static_cast<DesignerSettings*>(groupSettings())->formsSettingsChanged();
}

void FormsSettings::reset()
{
    guidelinesVisible = true;
    hoverOutlineVisible = true;
    backgroundColorMode = 0;
    outlineMode = 0;
    zoomLevel = 1.0;
    /****/
    gridViewVisible = true;
    snappingEnabled = true;
    gridSize = 8;
}

const char* FormsSettings::category() const
{
    return "Forms";
}

