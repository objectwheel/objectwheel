#include <interfacesettings.h>
#include <bootsettings.h>

#include <QTimer>

namespace {

const char* g_category = "Interface";
const char* g_leftBarColor = "LeftBarColor";
const char* g_topBarColor = "TopBarColor";
const char* g_theme = "Theme";
const char* g_language = "Language";
const char* g_hdpiEnabled = "HdpiEnabled";

inline QString joint(const char* setting)
{
    return QLatin1String(g_category) + "." + setting;
}
}

InterfaceSettings::InterfaceSettings(QObject* parent) : InterfaceSettings({}, parent)
{
}

InterfaceSettings::InterfaceSettings(const QString& group, QObject* parent) : Settings(group, parent)
{
    reset();
}

void InterfaceSettings::read()
{
    reset();

    QSettings* settings = BootSettings::settings();
    settings->beginGroup(group());
    theme = settings->value(joint(g_theme), theme).value<QString>();
    language = settings->value(joint(g_language), language).value<QString>();
    hdpiEnabled = settings->value(joint(g_hdpiEnabled), hdpiEnabled).value<bool>();
    topBarColor = settings->value(joint(g_topBarColor), topBarColor).value<QColor>();
    leftBarColor = settings->value(joint(g_leftBarColor), leftBarColor).value<QColor>();
    settings->endGroup();
}

void InterfaceSettings::write()
{
    QSettings* settings = BootSettings::settings();
    settings->beginGroup(group());
    settings->setValue(joint(g_theme), theme);
    settings->setValue(joint(g_language), language);
    settings->setValue(joint(g_hdpiEnabled), hdpiEnabled);
    settings->setValue(joint(g_topBarColor), topBarColor);
    settings->setValue(joint(g_leftBarColor), leftBarColor);
    settings->endGroup();
    emit changed();
}

void InterfaceSettings::reset()
{
    hdpiEnabled = true;
    theme = "Light";
    language = "English";
    topBarColor = "#247dd6";
    leftBarColor = "#3e474f";
}
