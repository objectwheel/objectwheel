#include <generalsettings.h>
#include <interfacesettings.h>

GeneralSettings* GeneralSettings::s_instance = nullptr;
InterfaceSettings* GeneralSettings::s_interfaceSettings = nullptr;

GeneralSettings::GeneralSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_interfaceSettings = new InterfaceSettings(this);
}

GeneralSettings::~GeneralSettings()
{
    s_instance = nullptr;
}

GeneralSettings* GeneralSettings::instance()
{
    return s_instance;
}

InterfaceSettings* GeneralSettings::interfaceSettings()
{
    return s_interfaceSettings;
}

const char* GeneralSettings::group() const
{
    return "General";
}

void GeneralSettings::read()
{
    s_interfaceSettings->read();
}

void GeneralSettings::write()
{
    s_interfaceSettings->write();
}

void GeneralSettings::reset()
{
    s_interfaceSettings->reset();
}
