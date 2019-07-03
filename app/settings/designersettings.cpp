#include <designersettings.h>
#include <scenesettings.h>

DesignerSettings* DesignerSettings::s_instance = nullptr;
SceneSettings* DesignerSettings::s_sceneSettings = nullptr;

DesignerSettings::DesignerSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_sceneSettings = new SceneSettings(this);
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

const char* DesignerSettings::group() const
{
    return "Designer";
}

void DesignerSettings::read()
{
    s_sceneSettings->read();
}

void DesignerSettings::write()
{
    s_sceneSettings->write();
}

void DesignerSettings::reset()
{
    s_sceneSettings->reset();
}
