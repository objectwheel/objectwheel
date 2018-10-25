#include <settings.h>
#include <groupsettings.h>
#include <applicationcore.h>
#include <QSettings>

Settings::Settings(GroupSettings* groupSettings) : m_groupSettings(groupSettings)
{
}

GroupSettings* Settings::groupSettings() const
{
    return m_groupSettings;
}

void Settings::begin() const
{
    ApplicationCore::settings()->beginGroup(groupSettings()->group());
}

void Settings::end() const
{
    ApplicationCore::settings()->endGroup();
}

void Settings::setValue(const char* setting, const QVariant& value)
{
    ApplicationCore::settings()->setValue(path(setting), value);
}

QVariant Settings::value(const char* setting, const QVariant& defaultValue)
{
    return ApplicationCore::settings()->value(path(setting), defaultValue);
}

QString Settings::path(const char* setting) const
{
    return category() + QLatin1Char('.') + setting;
}