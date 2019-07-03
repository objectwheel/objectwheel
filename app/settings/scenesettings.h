#ifndef SCENESETTINGS_H
#define SCENESETTINGS_H

#include <settings.h>

class DesignerSettings;
struct SceneSettings final : public Settings
{
    explicit SceneSettings(DesignerSettings* designerSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool showGuideLines;
    int sceneBackgroundColor;
    qreal sceneZoomLevel;
    /****/
    bool showGridViewDots;
    bool snappingEnabled;
    int gridSize;
    /****/
    bool showMouseoverOutline;
    int controlOutline;
};

#endif // SCENESETTINGS_H