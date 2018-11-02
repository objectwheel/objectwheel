#include <interfacesettings.h>
#include <generalsettings.h>

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
const char* g_preserveWindowStates = "PreserveWindowStates";
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
    hdpiEnabled = value<bool>(g_hdpiEnabled, hdpiEnabled);
    theme = value<QString>(g_theme, theme);
    language = value<QString>(g_language, language);
    topBarColor = value<QColor>(g_topBarColor, topBarColor);
    leftBarColor = value<QColor>(g_leftBarColor, leftBarColor);
    /****/
    fontPreferThick = value<bool>(g_fontPreferThick, fontPreferThick);
    fontPreferAntialiasing = value<bool>(g_fontPreferAntialiasing, fontPreferAntialiasing);
    fontPixelSize = value<int>(g_fontPixelSize, fontPixelSize);
    fontFamily = value<QString>(g_fontFamily, fontFamily);
    /****/
    bottomPanesPop = value<bool>(g_bottomPanesPop, bottomPanesPop);
    preserveWindowStates = value<bool>(g_preserveWindowStates, preserveWindowStates);
    visibleBottomPane = value<QString>(g_visibleBottomPane, visibleBottomPane);
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
    setValue(g_preserveWindowStates, preserveWindowStates);
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
    preserveWindowStates = true;
    visibleBottomPane = "None";
}

const char* InterfaceSettings::category() const
{
    return "Interface";
}

QFont InterfaceSettings::toFont() const
{
    QFont font(fontFamily);
    font.setPixelSize(fontPixelSize);
    font.setWeight(fontPreferThick ? QFont::Medium : QFont::Normal);
    font.setStyleStrategy(fontPreferAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    return font;
}
