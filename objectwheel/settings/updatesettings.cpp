#include <updatesettings.h>
#include <systemsettings.h>

static const char g_checkForUpdatesAutomatically[] = "CheckForUpdatesAutomatically";
static const char g_wereUpdatesAvailableLastTime[] = "WereUpdatesAvailableLastTime";
static const char g_lastSuccessfulUpdateCheckDate[] = "LastSuccessfulUpdateCheckDate";

UpdateSettings::UpdateSettings(SystemSettings* systemSettings) : Settings(systemSettings)
{
    reset();
}

void UpdateSettings::read()
{
    reset();

    begin();
    checkForUpdatesAutomatically = value<bool>(g_checkForUpdatesAutomatically, checkForUpdatesAutomatically);
    wereUpdatesAvailableLastTime = value<bool>(g_wereUpdatesAvailableLastTime, false);
    lastSuccessfulUpdateCheckDate = value<QDateTime>(g_lastSuccessfulUpdateCheckDate, QDateTime());
    end();
}

void UpdateSettings::write()
{
    begin();
    setValue(g_checkForUpdatesAutomatically, checkForUpdatesAutomatically);
    setValue(g_wereUpdatesAvailableLastTime, wereUpdatesAvailableLastTime);
    setValue(g_lastSuccessfulUpdateCheckDate, lastSuccessfulUpdateCheckDate);
    end();

    emit static_cast<SystemSettings*>(groupSettings())->updateSettingsChanged();
}

void UpdateSettings::reset()
{
    checkForUpdatesAutomatically = true;
    // wereUpdatesAvailableLastTime = false;
    // lastSuccessfulUpdateCheckDate = QDateTime();
}

const char* UpdateSettings::category() const
{
    return "Update";
}
