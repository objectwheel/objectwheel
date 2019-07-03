#ifndef INTERFACESETTINGS_H
#define INTERFACESETTINGS_H

#include <settings.h>
#include <QColor>
#include <QFont>

class GeneralSettings;

struct InterfaceSettings : public Settings
{
    friend class MainWindow;
    friend class WelcomeWindow;
    friend class PreferencesWindow;

    InterfaceSettings(GeneralSettings* generalSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    QFont toFont() const;

    bool hdpiEnabled;
    int theme;
    int language;
    /****/
    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
    /****/
    bool bottomPanesPop;
    bool preserveDesignerState;
    int visibleBottomPane;
};

#endif // INTERFACESETTINGS_H