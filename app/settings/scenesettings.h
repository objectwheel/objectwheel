#ifndef SCENESETTINGS_H
#define SCENESETTINGS_H

#include <settings.h>
#include <QColor>

class DesignerSettings;
struct SceneSettings final : public Settings
{
    explicit SceneSettings(DesignerSettings* designerSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;
    QBrush toBackgroundBrush() const;
    QBrush toBlankControlDecorationBrush() const;

    bool showGuideLines;
    int dragStartDistance;
    int sceneBackgroundTexture;
    qreal sceneZoomLevel;
    /****/
    bool showGridViewDots;
    bool snappingEnabled;
    int gridSize;
    /****/
    bool showMouseoverOutline;
    bool showClippedControls;
    int blankControlDecoration;
    int controlOutlineDecoration;
    QColor outlineColor;
};

#endif // SCENESETTINGS_H