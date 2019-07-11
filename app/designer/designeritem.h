#ifndef DESIGNERITEM_H
#define DESIGNERITEM_H

#include <QGraphicsObject>
#include <QPen>

class Control;
class DesignerScene;

class DesignerItem : public QGraphicsObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerItem)

public:
    explicit DesignerItem(Control* parent = nullptr);

    Control* parentControl() const;
    DesignerScene* scene() const;

    QPen pen() const;
    void setPen(const QPen& pen);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

protected:
    bool dragStarted() const;
    QPointF snapPosition() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void doubleClicked();

private:
    QPen m_pen;
    QBrush m_brush;
    bool m_dragStarted;
    QPointF m_snapPosition;
    QPointF m_dragStartPoint;
};

#endif // DESIGNERITEM_H
