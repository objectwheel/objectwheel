#ifndef INTERFACESETTINGS_H
#define INTERFACESETTINGS_H

#include <settings.h>
#include <QColor>

class GeneralSettings;

struct InterfaceSettings : public Settings
{
    InterfaceSettings(GeneralSettings* generalSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool hdpiEnabled;
    QString theme;
    QString language;
    QColor topBarColor;
    QColor leftBarColor;
    /****/
    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
    /****/
    bool bottomPanesPop;
    QString visibleBottomPane;
};

#endif // INTERFACESETTINGS_H