#ifndef FORMSSETTINGS_H
#define FORMSSETTINGS_H

#include <settings.h>
#include <QColor>
#include <QFont>

class DesignerSettings;

struct FormsSettings : public Settings
{
    friend class MainWindow;
    friend class WelcomeWindow;
    friend class PreferencesWindow;

    FormsSettings(DesignerSettings* designerSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    QFont toFont() const;

    bool hdpiEnabled;
    QString theme;
    QString language;
    /****/
    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
    /****/
    bool bottomPanesPop;
    bool preserveDesignerState;
    QString visibleBottomPane;
};

#endif // FORMSSETTINGS_H