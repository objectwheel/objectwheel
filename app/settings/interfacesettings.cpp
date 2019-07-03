#include <interfacesettings.h>
#include <generalsettings.h>

static const char g_theme[] = "Theme";
static const char g_fontFamily[] = "FontFamily";
static const char g_fontPixelSize[] = "FontPixelSize";
static const char g_fontPreferThick[] = "FontPreferThick";
static const char g_fontPreferAntialiasing[] = "FontPreferAntialiasing";
static const char g_language[] = "Language";
static const char g_hdpiEnabled[] = "HdpiEnabled";
static const char g_bottomPanesPop[] = "BottomPanesPop";
static const char g_preserveDesignerState[] = "PreserveDesignerState";
static const char g_visibleBottomPane[] = "VisibleBottomPane";

InterfaceSettings::InterfaceSettings(GeneralSettings* generalSettings) : Settings(generalSettings)
{
    reset();
}

void InterfaceSettings::read()
{
    reset();

    begin();
    hdpiEnabled = value<bool>(g_hdpiEnabled, hdpiEnabled);
    theme = value<int>(g_theme, theme);
    language = value<int>(g_language, language);
    /****/
    fontPreferThick = value<bool>(g_fontPreferThick, fontPreferThick);
    fontPreferAntialiasing = value<bool>(g_fontPreferAntialiasing, fontPreferAntialiasing);
    fontPixelSize = value<int>(g_fontPixelSize, fontPixelSize);
    fontFamily = value<QString>(g_fontFamily, fontFamily);
    /****/
    bottomPanesPop = value<bool>(g_bottomPanesPop, bottomPanesPop);
    preserveDesignerState = value<bool>(g_preserveDesignerState, preserveDesignerState);
    visibleBottomPane = value<int>(g_visibleBottomPane, visibleBottomPane);
    end();
}

void InterfaceSettings::write()
{
    begin();
    setValue(g_hdpiEnabled, hdpiEnabled);
    setValue(g_theme, theme);
    setValue(g_language, language);
    /****/
    setValue(g_fontPreferThick, fontPreferThick);
    setValue(g_fontPreferAntialiasing, fontPreferAntialiasing);
    setValue(g_fontPixelSize, fontPixelSize);
    setValue(g_fontFamily, fontFamily);
    /****/
    setValue(g_bottomPanesPop, bottomPanesPop);
    setValue(g_preserveDesignerState, preserveDesignerState);
    setValue(g_visibleBottomPane, visibleBottomPane);
    end();

    emit static_cast<GeneralSettings*>(groupSettings())->interfaceSettingsChanged();
}

void InterfaceSettings::reset()
{
    hdpiEnabled = true;
    theme = 0;
    language = 0;
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
    preserveDesignerState = true;
    visibleBottomPane = 0;
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
