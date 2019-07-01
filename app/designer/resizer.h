#ifndef RESIZER_H
#define RESIZER_H

#include <QGraphicsRectItem>

class Control;
class Resizer final : public QGraphicsRectItem
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

    Control* parentControl() const;

    Placement placement() const;
    void setPlacement(Placement placement);

    void updateCursor();
    void updatePosition();

    static QList<Resizer*> init(Control* control);

private:
    void calculatePositionDifference(const QGraphicsSceneMouseEvent* event, qreal* dx, qreal* dy);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Placement m_placement;
};

#endif // RESIZER_H
