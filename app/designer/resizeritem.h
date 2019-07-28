#ifndef RESIZERITEM_H
#define RESIZERITEM_H

#include <gadgetitem.h>
#include <QPointer>

class ResizerItem final : public GadgetItem
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

    DesignerItem* targetItem() const;
    void setTargetItem(DesignerItem* targetItem);

    void updateCursor();
    void updatePosition();
    void updateTargetGeometry();

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void ungrabMouseEvent(QEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Placement m_placement;
    QPointer<DesignerItem> m_targetItem;
};

#endif // RESIZERITEM_H
