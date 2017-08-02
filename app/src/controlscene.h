#ifndef CONTROLSCENE_H
#define CONTROLSCENE_H

#include <control.h>

#include <QGraphicsScene>
#include <QPointer>

class ControlScenePrivate;

class ControlScene : public QGraphicsScene
{
        Q_OBJECT
        Q_DISABLE_COPY(ControlScene)
        friend class ControlScenePrivate;

    public:
        explicit ControlScene(QObject *parent = Q_NULLPTR);
        static ControlScene* instance();

        static Control* currentControl();
        static void setCurrentControl(Control* currentControl);

        static void removeControl(Control* control);
        static void removeChildControlsOnly(Control* parent);
        static QList<Control*> controls(Qt::SortOrder order = Qt::DescendingOrder);
        static QList<Control*> selectedControls();

        static bool showOutlines();
        static void setShowOutlines(bool value);

        static bool snapping();
        static void setSnapping(bool snapping);

        static QPointF lastMousePos();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        void drawForeground(QPainter *painter, const QRectF &rect) override;

    private:
        static ControlScenePrivate* _d;
        static QPointer<Control> _currentControl;
        static bool _snapping;
        static QPointF _lastMousePos;

};

#endif // CONTROLSCENE_H
