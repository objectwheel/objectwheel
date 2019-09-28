#include <toolboxsettings.h>
#include <designersettings.h>

static const char g_enableAlternatingRowColors[] = "EnableAlternatingRowColors";
static const char g_textElideMode[] = "TextElideMode";
static const char g_iconSize[] = "IconSize";

ToolboxSettings::ToolboxSettings(DesignerSettings* designerSettings) : Settings(designerSettings)
{
    reset();
}

void ToolboxSettings::read()
{
    reset();

    begin();
    enableAlternatingRowColors = value<bool>(g_enableAlternatingRowColors, enableAlternatingRowColors);
    textElideMode = value<int>(g_textElideMode, textElideMode);
    iconSize = value<int>(g_iconSize, iconSize);
    end();
}

void ToolboxSettings::write()
{
    begin();
    setValue(g_enableAlternatingRowColors, enableAlternatingRowColors);
    setValue(g_textElideMode, textElideMode);
    setValue(g_iconSize, iconSize);
    end();

    emit static_cast<DesignerSettings*>(groupSettings())->toolboxSettingsChanged();
}

void ToolboxSettings::reset()
{
    enableAlternatingRowColors = true;
    textElideMode = 2;
    iconSize = 16;
}

const char* ToolboxSettings::category() const
{
    return "Toolbox";
}
