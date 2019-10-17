#include <navigatorsettings.h>
#include <designersettings.h>

static const char g_itemDoubleClickAction[] = "ItemDoubleClickAction";

NavigatorSettings::NavigatorSettings(DesignerSettings* designerSettings) : Settings(designerSettings)
{
    reset();
}

void NavigatorSettings::read()
{
    reset();

    begin();
    itemDoubleClickAction = value<int>(g_itemDoubleClickAction, itemDoubleClickAction);
    end();
}

void NavigatorSettings::write()
{
    begin();
    setValue(g_itemDoubleClickAction, itemDoubleClickAction);
    end();

    emit static_cast<DesignerSettings*>(groupSettings())->navigatorSettingsChanged();
}

void NavigatorSettings::reset()
{
    itemDoubleClickAction = 2;
}

const char* NavigatorSettings::category() const
{
    return "Navigator";
}
