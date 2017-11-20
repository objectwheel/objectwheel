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
        explicit Resizer(QGraphicsWidget* parent = nullptr);
        Placement placement() const;
        void setPlacement(Placement placement);
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
        virtual void paint(QPainter* painter, const QSogi* option,
          QWidget* widget = nullptr) override;

    private:
        Placement _placement;
        bool _disabled;
        static bool _resizing;
};

#endif // RESIZER_H
