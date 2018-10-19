#include <interfacesettings.h>
#include <usermanager.h>

#include <QSettings>
#include <QTimer>

namespace {

const char* g_category = "InterfaceSettings";
const char* g_color = "Color";
const char* g_theme = "Theme";
const char* g_language = "Language";
const char* g_hdpiEnabled = "HdpiEnabled";

inline QString joint(const char* setting)
{
    return QLatin1String(g_category) + "." + setting;
}
}

InterfaceSettings::InterfaceSettings(const QString& group) : Settings(group)
  , color("#3e474f")
  , theme("Light")
  , language("English")
  , hdpiEnabled(true)
{
}

void InterfaceSettings::read()
{
    reset();

    QSettings* settings = UserManager::settings();
    settings->beginGroup(group());
    color = settings->value(joint(g_color), color).value<QColor>();
    theme = settings->value(joint(g_theme), theme).value<QString>();
    language = settings->value(joint(g_language), language).value<QString>();
    hdpiEnabled = settings->value(joint(g_hdpiEnabled), hdpiEnabled).value<bool>();
    settings->endGroup();
}

void InterfaceSettings::write()
{
    QSettings* settings = UserManager::settings();
    settings->beginGroup(group());
    settings->setValue(joint(g_color), color);
    settings->setValue(joint(g_theme), theme);
    settings->setValue(joint(g_language), language);
    settings->setValue(joint(g_hdpiEnabled), hdpiEnabled);
    settings->endGroup();
}

void InterfaceSettings::reset()
{
    *this = InterfaceSettings(group());
}
