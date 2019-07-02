#ifndef FORMSSETTINGS_H
#define FORMSSETTINGS_H

#include <settings.h>
#include <QColor>
#include <QFont>

class DesignerSettings;

struct FormsSettings : public Settings
{
    FormsSettings(DesignerSettings* designerSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool guidelinesVisible;
    bool hoverOutlineVisible;
    int backgroundColorMode;
    int outlineMode;
    qreal zoomLevel;
    /****/
    bool gridViewVisible;
    bool snappingEnabled;
    int gridSize;
};

#endif // FORMSSETTINGS_H