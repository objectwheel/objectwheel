#ifndef GADGETITEM_H
#define GADGETITEM_H

#include <designeritem.h>
#include <QPen>

class GadgetItem : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(GadgetItem)

public:
    explicit GadgetItem(DesignerItem* parent = nullptr);

    QPen pen() const;
    void setPen(const QPen& pen);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

protected:
    QPointF dragDistanceVector() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseUngrabEvent(QEvent* event) override;

private:
    QPen m_pen;
    QBrush m_brush;
    QPointF m_dragDistanceVector;
};

#endif // GADGETITEM_H
