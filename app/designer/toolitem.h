#ifndef TOOLITEM_H
#define TOOLITEM_H

#include <designeritem.h>

class ToolItem : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolItem)

public:
    explicit ToolItem(DesignerItem* parent = nullptr);

    QPen pen() const;
    void setPen(const QPen& pen);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

protected:
    QPointF dragDistanceVector() const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QPen m_pen;
    QBrush m_brush;
    QPointF m_dragDistanceVector;
};

#endif // TOOLITEM_H
