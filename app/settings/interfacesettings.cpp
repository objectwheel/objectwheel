#include <interfacesettings.h>
#include <applicationcore.h>

#include <QSettings>
#include <QFontInfo>

namespace {

const char* g_category = "Interface";
const char* g_leftBarColor = "LeftBarColor";
const char* g_topBarColor = "TopBarColor";
const char* g_theme = "Theme";
const char* g_fontFamily = "FontFamily";
const char* g_fontPixelSize = "FontPixelSize";
const char* g_fontPreferThick = "FontPreferThick";
const char* g_fontPreferAntialiasing = "FontPreferAntialiasing";
const char* g_language = "Language";
const char* g_hdpiEnabled = "HdpiEnabled";
const char* g_bottomPanesPop = "BottomPanesPop";
const char* g_visibleBottomPane = "VisibleBottomPane";

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

    QSettings* settings = ApplicationCore::settings();
    settings->beginGroup(group());
    theme = settings->value(joint(g_theme), theme).value<QString>();

    fontFamily = settings->value(joint(g_fontFamily), fontFamily).value<QString>();
    fontPixelSize = settings->value(joint(g_fontPixelSize), fontFamily).value<int>();
    fontPreferThick = settings->value(joint(g_fontPreferThick), fontPreferThick).value<bool>();
    fontPreferAntialiasing = settings->value(joint(g_fontPreferAntialiasing), fontPreferAntialiasing).value<bool>();

    language = settings->value(joint(g_language), language).value<QString>();
    hdpiEnabled = settings->value(joint(g_hdpiEnabled), hdpiEnabled).value<bool>();
    bottomPanesPop = settings->value(joint(g_bottomPanesPop), bottomPanesPop).value<bool>();
    topBarColor = settings->value(joint(g_topBarColor), topBarColor).value<QColor>();
    leftBarColor = settings->value(joint(g_leftBarColor), leftBarColor).value<QColor>();
    visibleBottomPane = settings->value(joint(g_visibleBottomPane), visibleBottomPane).value<QString>();
    settings->endGroup();
}

void InterfaceSettings::write()
{
    QSettings* settings = ApplicationCore::settings();
    settings->beginGroup(group());
    settings->setValue(joint(g_theme), theme);
    settings->setValue(joint(g_font), font);
    settings->setValue(joint(g_language), language);
    settings->setValue(joint(g_hdpiEnabled), hdpiEnabled);
    settings->setValue(joint(g_bottomPanesPop), bottomPanesPop);
    settings->setValue(joint(g_topBarColor), topBarColor);
    settings->setValue(joint(g_leftBarColor), leftBarColor);
    settings->setValue(joint(g_visibleBottomPane), visibleBottomPane);
    settings->endGroup();
    emit changed();
}

void InterfaceSettings::reset()
{
#if defined(Q_OS_MACOS)
    fontFamily = ".SF NS Display";
#elif defined(Q_OS_WIN)
    fontFamily = "Segoe UI";
#else
    fontFamily = "Roboto";
#endif

    hdpiEnabled = true;
    bottomPanesPop = false;
    fontPreferThick = false;
    fontPreferAntialiasing = true;
    fontPixelSize = 13;
    theme = "Light";
    language = "English";
    topBarColor = "#247dd6";
    leftBarColor = "#3e474f";
    visibleBottomPane = "None";
}
