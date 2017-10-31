#ifndef CONTROLSCENE_H
#define CONTROLSCENE_H

#include <control.h>
#include <QGraphicsScene>
#include <QPointer>

class ControlScenePrivate;

class ControlScene : public QGraphicsScene
{
        Q_OBJECT
        friend class ControlScenePrivate;

    public:
        explicit ControlScene(QObject *parent = Q_NULLPTR);

        Control* mainControl();
        virtual void setMainControl(Control* mainControl);

        void removeControl(Control* control);
        void removeChildControlsOnly(Control* parent);
        QList<Control*> controls(Qt::SortOrder order = Qt::DescendingOrder);
        QList<Control*> selectedControls();

        bool showOutlines();
        void setShowOutlines(bool value);

        bool snapping();
        void setSnapping(bool snapping);

        QPointF lastMousePos();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        void drawForeground(QPainter *painter, const QRectF &rect) override;

    signals:
        void aboutToRemove(Control* control);
        void controlRemoved(Control* control);
        void mainControlChanged(Control* mainControl);

    protected:
        QPointer<Control> _mainControl;

    private:
        ControlScenePrivate* _d;
        bool _snapping;
        QPointF _lastMousePos;
};

#endif // CONTROLSCENE_H
