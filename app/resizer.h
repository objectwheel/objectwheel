#ifndef RESIZER_H
#define RESIZER_H

#include <QGraphicsWidget>

typedef QGraphicsSceneMouseEvent QGsme;
typedef QStyleOptionGraphicsItem QSogi;

class Resizer : public QGraphicsWidget
{
        Q_OBJECT

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
        explicit Resizer(QGraphicsWidget* parent, Placement placement);
        bool disabled() const;
        void setDisabled(bool disabled);
        static bool resizing();

    public slots:
        void correct();

    protected:
        virtual QRectF boundingRect() const override;
        virtual void mousePressEvent(QGsme* event) override;
        virtual void mouseMoveEvent(QGsme* event) override;
        virtual void mouseReleaseEvent(QGsme* event) override;
        virtual void paint(QPainter* painter, const QSogi* option, QWidget* widget = nullptr) override;

    private:
        Placement m_placement;
        bool m_disabled;
        static bool m_resizing;
};

#endif // RESIZER_H
