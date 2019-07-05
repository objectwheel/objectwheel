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
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Placement m_placement;
};

#endif // RESIZER_H
