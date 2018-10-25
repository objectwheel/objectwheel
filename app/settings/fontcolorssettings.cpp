#include <fontcolorssettings.h>
#include <applicationcore.h>
#include <QSettings>

namespace {

const char* g_category = "FontColors";
const char* g_fontFamily = "FontFamily";
const char* g_fontPixelSize = "FontPixelSize";
const char* g_fontPreferThick = "FontPreferThick";
const char* g_fontPreferAntialiasing = "FontPreferAntialiasing";

inline QString joint(const char* setting)
{
    return QLatin1String(g_category) + "." + setting;
}
}

FontColorsSettings::FontColorsSettings(QObject* parent) : FontColorsSettings({}, parent)
{
}

FontColorsSettings::FontColorsSettings(const QString& group, QObject* parent) : Settings(group, parent)
{
    reset();
}

void FontColorsSettings::read()
{
    reset();

    QSettings* settings = ApplicationCore::settings();
    settings->beginGroup(group());
    /****/
    fontFamily = settings->value(joint(g_fontFamily), fontFamily).value<QString>();
    fontPixelSize = settings->value(joint(g_fontPixelSize), fontPixelSize).value<int>();
    fontPreferThick = settings->value(joint(g_fontPreferThick), fontPreferThick).value<bool>();
    fontPreferAntialiasing = settings->value(joint(g_fontPreferAntialiasing), fontPreferAntialiasing).value<bool>();
    /****/
    settings->endGroup();
}

void FontColorsSettings::write()
{
    QSettings* settings = ApplicationCore::settings();
    settings->beginGroup(group());
    /****/
    settings->setValue(joint(g_fontFamily), fontFamily);
    settings->setValue(joint(g_fontPixelSize), fontPixelSize);
    settings->setValue(joint(g_fontPreferThick), fontPreferThick);
    settings->setValue(joint(g_fontPreferAntialiasing), fontPreferAntialiasing);
    /****/
    settings->endGroup();
    emit changed();
}

void FontColorsSettings::reset()
{
#if defined(Q_OS_MACOS)
    fontFamily = ".SF NS Display";
#elif defined(Q_OS_WIN)
    fontFamily = "Segoe UI";
#else
    fontFamily = "Roboto";
#endif
    fontPreferThick = false;
    fontPreferAntialiasing = true;
    fontPixelSize = 13;
}
