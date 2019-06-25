#include <quicktheme.h>
#include <saveutils.h>
#include <QJsonObject>
#include <QJsonDocument>

namespace QuickTheme {

void setTheme(const QString& projectDir, int* version)
{
    const QJsonObject& object = QJsonDocument::fromBinaryData(SaveUtils::projectTheme(projectDir)).object();
    const QString& stylev1 = object.value("stylev1").toString();
    const QString& stylev2 = object.value("stylev2").toString();
    const QString& theme = object.value("theme").toString();
    const QString& accent = object.value("accent").toString();
    const QString& primary = object.value("primary").toString();
    const QString& foreground = object.value("foreground").toString();
    const QString& background = object.value("background").toString();

    qputenv("QT_QUICK_CONTROLS_1_STYLE", stylev1.toUtf8());
    qputenv("QT_QUICK_CONTROLS_STYLE", stylev2.toUtf8());
    qputenv("QT_LABS_CONTROLS_STYLE", stylev2.toUtf8());

    if (stylev2 == "Material") {
        if (!theme.isEmpty())
            qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", theme.toUtf8());

        if (!accent.isEmpty())
            qputenv("QT_QUICK_CONTROLS_MATERIAL_ACCENT", accent.toUtf8());

        if (!primary.isEmpty())
            qputenv("QT_QUICK_CONTROLS_MATERIAL_PRIMARY", primary.toUtf8());

        if (!foreground.isEmpty())
            qputenv("QT_QUICK_CONTROLS_MATERIAL_FOREGROUND", foreground.toUtf8());

        if (!background.isEmpty())
            qputenv("QT_QUICK_CONTROLS_MATERIAL_BACKGROUND", background.toUtf8());
    } else if (stylev2 == "Universal") {
        if (!theme.isEmpty())
            qputenv("QT_QUICK_CONTROLS_UNIVERSAL_THEME", theme.toUtf8());

        if (!accent.isEmpty())
            qputenv("QT_QUICK_CONTROLS_UNIVERSAL_ACCENT", accent.toUtf8());

        if (!foreground.isEmpty())
            qputenv("QT_QUICK_CONTROLS_UNIVERSAL_FOREGROUND", foreground.toUtf8());

        if (!background.isEmpty())
            qputenv("QT_QUICK_CONTROLS_UNIVERSAL_BACKGROUND", background.toUtf8());
    }

    if (version && (!stylev1.isEmpty() || !stylev2.isEmpty()))
        *version = stylev1.isEmpty() ? 2 : 1;
}
}