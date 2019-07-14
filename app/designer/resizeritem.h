#ifndef RESIZERITEM_H
#define RESIZERITEM_H

#include <designeritem.h>

class ResizerItem final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(ResizerItem)

public:
    enum Placement {
        Top, Right, Bottom, Left, TopLeft,
        TopRight, BottomRight, BottomLeft
    };

public:
    explicit ResizerItem(Placement placement, DesignerItem* parent = nullptr);
    void updateCursor();
    void updatePosition();

private:
    void setParentGeometry(const QPointF& dragDistance);

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Placement m_placement;
};

#endif // RESIZERITEM_H
