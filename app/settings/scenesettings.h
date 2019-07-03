#ifndef SCENESETTINGS_H
#define SCENESETTINGS_H

#include <settings.h>
#include <QColor>
#include <QFont>

class DesignerSettings;

struct SceneSettings : public Settings
{
    SceneSettings(DesignerSettings* designerSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool showGuideLines;
    int sceneBackgroundColorMode;
    qreal sceneZoomLevel;
    /****/
    bool showGridViewDots;
    bool snappingEnabled;
    int gridSize;
    /****/
    bool showMouseoverOutline;
    int controlOutlineMode;
};

#endif // SCENESETTINGS_H