#ifndef INTERFACESETTINGS_H
#define INTERFACESETTINGS_H

#include <settings.h>
#include <QFont>
#include <QColor>

class GeneralSettings;
struct InterfaceSettings final : public Settings
{
    explicit InterfaceSettings(GeneralSettings* generalSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;
    QFont toFont() const;

    bool hdpiEnabled;
    int theme;
    int language;
    QColor highlightColor;
    /****/
    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
    /****/
    bool outputPanePops;
    bool preserveDesignerState;
    int visibleOutputWidget;
};

#endif // INTERFACESETTINGS_H