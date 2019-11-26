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
    QBrush toBlankControlDecorationBrush(const QColor& color) const;

    bool showGuideLines;
    bool showAllAnchors;
    int dragStartDistance;
    int sceneBackgroundTexture;
    qreal sceneZoomLevel;
    QColor anchorColor;
    /****/
    bool showGridViewDots;
    bool snappingEnabled;
    int gridSize;
    /****/
    bool showMouseoverOutline;
    bool showClippedControls;
    int blankControlDecoration;
    int controlOutlineDecoration;
    int controlDoubleClickAction;
};

#endif // SCENESETTINGS_H