#include <interfacesettings.h>
#include <generalsettings.h>
#include <QVariant>

namespace {
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
}

InterfaceSettings::InterfaceSettings(GeneralSettings* generalSettings) : Settings(generalSettings)
{
    reset();
}

void InterfaceSettings::read()
{
    reset();

    begin();
    hdpiEnabled = value(g_hdpiEnabled, hdpiEnabled).value<bool>();
    theme = value(g_theme, theme).value<QString>();
    language = value(g_language, language).value<QString>();
    topBarColor = value(g_topBarColor, topBarColor).value<QColor>();
    leftBarColor = value(g_leftBarColor, leftBarColor).value<QColor>();
    /****/
    fontPreferThick = value(g_fontPreferThick, fontPreferThick).value<bool>();
    fontPreferAntialiasing = value(g_fontPreferAntialiasing, fontPreferAntialiasing).value<bool>();
    fontPixelSize = value(g_fontPixelSize, fontPixelSize).value<int>();
    fontFamily = value(g_fontFamily, fontFamily).value<QString>();
    /****/
    bottomPanesPop = value(g_bottomPanesPop, bottomPanesPop).value<bool>();
    visibleBottomPane = value(g_visibleBottomPane, visibleBottomPane).value<QString>();
    end();
}

void InterfaceSettings::write()
{
    begin();
    setValue(g_hdpiEnabled, hdpiEnabled);
    setValue(g_theme, theme);
    setValue(g_language, language);
    setValue(g_topBarColor, topBarColor);
    setValue(g_leftBarColor, leftBarColor);
    /****/
    setValue(g_fontPreferThick, fontPreferThick);
    setValue(g_fontPreferAntialiasing, fontPreferAntialiasing);
    setValue(g_fontPixelSize, fontPixelSize);
    setValue(g_fontFamily, fontFamily);
    /****/
    setValue(g_bottomPanesPop, bottomPanesPop);
    setValue(g_visibleBottomPane, visibleBottomPane);
    end();

    emit static_cast<GeneralSettings*>(groupSettings())->interfaceSettingsChanged();
}

void InterfaceSettings::reset()
{
    hdpiEnabled = true;
    theme = "Light";
    language = "English";
    topBarColor = "#247dd6";
    leftBarColor = "#3e474f";
    /****/
    fontPreferThick = false;
    fontPreferAntialiasing = true;
    fontPixelSize = 13;
#if defined(Q_OS_MACOS)
    fontFamily = ".SF NS Display";
#elif defined(Q_OS_WIN)
    fontFamily = "Segoe UI";
#else
    fontFamily = "Roboto";
#endif
    /****/
    bottomPanesPop = false;
    visibleBottomPane = "None";
}

const char* InterfaceSettings::category() const
{
    return "Interface";
}
