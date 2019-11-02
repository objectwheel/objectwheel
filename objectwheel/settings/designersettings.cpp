#include <designersettings.h>
#include <scenesettings.h>
#include <toolboxsettings.h>
#include <controlssettings.h>

DesignerSettings* DesignerSettings::s_instance = nullptr;
SceneSettings* DesignerSettings::s_sceneSettings = nullptr;
ToolboxSettings* DesignerSettings::s_toolboxSettings = nullptr;
ControlsSettings* DesignerSettings::s_controlsSettings = nullptr;

DesignerSettings::DesignerSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_sceneSettings = new SceneSettings(this);
    s_toolboxSettings = new ToolboxSettings(this);
    s_controlsSettings = new ControlsSettings(this);
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

ControlsSettings* DesignerSettings::controlsSettings()
{
    return s_controlsSettings;
}

const char* DesignerSettings::group() const
{
    return "Designer";
}

void DesignerSettings::read()
{
    s_sceneSettings->read();
    s_toolboxSettings->read();
    s_controlsSettings->read();
}

void DesignerSettings::write()
{
    s_sceneSettings->write();
    s_toolboxSettings->write();
    s_controlsSettings->write();
}

void DesignerSettings::reset()
{
    s_sceneSettings->reset();
    s_toolboxSettings->reset();
    s_controlsSettings->reset();
}
