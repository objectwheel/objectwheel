#include <designersettings.h>
#include <formssettings.h>

DesignerSettings* DesignerSettings::s_instance = nullptr;
FormsSettings* DesignerSettings::s_formsSettings = nullptr;

DesignerSettings::DesignerSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_formsSettings = new FormsSettings(this);
}

DesignerSettings::~DesignerSettings()
{
    s_instance = nullptr;
}

DesignerSettings* DesignerSettings::instance()
{
    return s_instance;
}

FormsSettings* DesignerSettings::formsSettings()
{
    return s_formsSettings;
}

const char* DesignerSettings::group() const
{
    return "Designer";
}

void DesignerSettings::read()
{
    s_formsSettings->read();
}

void DesignerSettings::write()
{
    s_formsSettings->write();
}

void DesignerSettings::reset()
{
    s_formsSettings->reset();
}
