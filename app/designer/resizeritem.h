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
    explicit ResizerItem(Placement placement, Control* parent);
    static QList<ResizerItem*> init(Control* control);
    void updateCursor();
    void updatePosition();

private:
    QRectF calculateParentGeometry(const QPointF& snapPos);

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Placement m_placement;
};

#endif // RESIZERITEM_H
