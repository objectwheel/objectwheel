#include <generalsettings.h>
#include <interfacesettings.h>
#include <updatesettings.h>

GeneralSettings* GeneralSettings::s_instance = nullptr;
InterfaceSettings* GeneralSettings::s_interfaceSettings = nullptr;
UpdateSettings* GeneralSettings::s_updateSettings = nullptr;

GeneralSettings::GeneralSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_interfaceSettings = new InterfaceSettings(this);
    s_updateSettings = new UpdateSettings(this);
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

UpdateSettings* GeneralSettings::updateSettings()
{
    return s_updateSettings;
}

const char* GeneralSettings::group() const
{
    return "General";
}

void GeneralSettings::read()
{
    s_interfaceSettings->read();
    s_updateSettings->read();
}

void GeneralSettings::write()
{
    s_interfaceSettings->write();
    s_updateSettings->write();
}

void GeneralSettings::reset()
{
    s_interfaceSettings->reset();
    s_updateSettings->reset();
}
