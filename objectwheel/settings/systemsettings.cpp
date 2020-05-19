#include <systemsettings.h>
#include <updatesettings.h>

SystemSettings* SystemSettings::s_instance = nullptr;
UpdateSettings* SystemSettings::s_updateSettings = nullptr;

SystemSettings::SystemSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_updateSettings = new UpdateSettings(this);
}

SystemSettings::~SystemSettings()
{
    s_instance = nullptr;
}

SystemSettings* SystemSettings::instance()
{
    return s_instance;
}

UpdateSettings* SystemSettings::updateSettings()
{
    return s_updateSettings;
}

const char* SystemSettings::group() const
{
    return "System";
}

void SystemSettings::read()
{
    s_updateSettings->read();
}

void SystemSettings::write()
{
    s_updateSettings->write();
}

void SystemSettings::reset()
{
    s_updateSettings->reset();
}
