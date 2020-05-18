#include <updatesettings.h>
#include <generalsettings.h>

static const char g_checkForUpdatesAutomatically[] = "CheckForUpdatesAutomatically";
static const char g_lastUpdateCheckDate[] = "LastUpdateCheckDate";

UpdateSettings::UpdateSettings(GeneralSettings* generalSettings) : Settings(generalSettings)
{
    reset();
}

void UpdateSettings::read()
{
    reset();

    begin();
    checkForUpdatesAutomatically = value<bool>(g_checkForUpdatesAutomatically, checkForUpdatesAutomatically);
    lastUpdateCheckDate = value<QDateTime>(g_lastUpdateCheckDate, lastUpdateCheckDate);
    end();
}

void UpdateSettings::write()
{
    begin();
    setValue(g_checkForUpdatesAutomatically, checkForUpdatesAutomatically);
    setValue(g_lastUpdateCheckDate, lastUpdateCheckDate);
    end();

    emit static_cast<GeneralSettings*>(groupSettings())->updateSettingsChanged();
}

void UpdateSettings::reset()
{
    checkForUpdatesAutomatically = true;
    // lastUpdateCheckDate = QDateTime();
}

const char* UpdateSettings::category() const
{
    return "Update";
}
