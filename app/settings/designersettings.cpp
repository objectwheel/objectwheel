#include <designersettings.h>
#include <scenesettings.h>
#include <toolboxsettings.h>

DesignerSettings* DesignerSettings::s_instance = nullptr;
SceneSettings* DesignerSettings::s_sceneSettings = nullptr;
ToolboxSettings* DesignerSettings::s_toolboxSettings = nullptr;

DesignerSettings::DesignerSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_sceneSettings = new SceneSettings(this);
    s_toolboxSettings = new ToolboxSettings(this);
}

DesignerSettings::~DesignerSettings()
{
    s_instance = nullptr;
}

DesignerSettings* DesignerSettings::instance()
{
    return s_instance;
}

SceneSettings* DesignerSettings::sceneSettings()
{
    return s_sceneSettings;
}

ToolboxSettings* DesignerSettings::toolboxSettings()
{
    return s_toolboxSettings;
}

const char* DesignerSettings::group() const
{
    return "Designer";
}

void DesignerSettings::read()
{
    s_sceneSettings->read();
    s_toolboxSettings->read();
}

void DesignerSettings::write()
{
    s_sceneSettings->write();
    s_toolboxSettings->write();
}

void DesignerSettings::reset()
{
    s_sceneSettings->reset();
    s_toolboxSettings->reset();
}
