#ifndef RESIZER_H
#define RESIZER_H

#include <QGraphicsItem>

class Control;
class DesignerScene;

class Resizer final : public QGraphicsItem
{
    Q_DISABLE_COPY(Resizer)

public:
    enum Placement {
        Top,
        Right,
        Bottom,
        Left,
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft
    };

public:
    explicit Resizer(Placement placement, Control* parent);

    static QList<Resizer*> init(Control* control);

    DesignerScene* scene() const;
    Control* parentControl() const;
    QRectF boundingRect() const override;

    void updateCursor();
    void updatePosition();

private:
    QRectF calculateParentGeometry(const QPointF& snapPos);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Placement m_placement;
    QPointF m_dragStartPoint;
};

#endif // RESIZER_H
