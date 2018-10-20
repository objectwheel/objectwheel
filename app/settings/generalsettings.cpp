#include <generalsettings.h>
#include <interfacesettings.h>

namespace { const char* g_group = "General"; }

GeneralSettings* GeneralSettings::s_instance = nullptr;
InterfaceSettings* GeneralSettings::s_interfaceSettings = nullptr;

GeneralSettings::GeneralSettings(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_interfaceSettings = new InterfaceSettings(g_group, this);
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
