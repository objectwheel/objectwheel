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

    DesignerScene* scene() const;

    Control* parentControl() const;

    void updateCursor();
    void updatePosition();

    QRectF boundingRect() const override;

    static QList<Resizer*> init(Control* control);

private:
    void calculatePositionDifference(const QGraphicsSceneMouseEvent* event, qreal* dx, qreal* dy);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    qreal m_collectiveDx, m_collectiveDy;
    Placement m_placement;
};

#endif // RESIZER_H
