#include <controlssettings.h>
#include <designersettings.h>

static const char g_itemDoubleClickAction[] = "ItemDoubleClickAction";

ControlsSettings::ControlsSettings(DesignerSettings* designerSettings) : Settings(designerSettings)
{
    reset();
}

void ControlsSettings::read()
{
    reset();

    begin();
    itemDoubleClickAction = value<int>(g_itemDoubleClickAction, itemDoubleClickAction);
    end();
}

void ControlsSettings::write()
{
    begin();
    setValue(g_itemDoubleClickAction, itemDoubleClickAction);
    end();

    emit static_cast<DesignerSettings*>(groupSettings())->controlsSettingsChanged();
}

void ControlsSettings::reset()
{
    itemDoubleClickAction = 2;
}

const char* ControlsSettings::category() const
{
    return "Controls";
}
