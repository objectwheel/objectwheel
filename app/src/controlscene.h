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

        Control* mainControl() const;
        virtual void setMainControl(Control* mainControl);

        void removeControl(Control* control);
        void removeChildControlsOnly(Control* parent);
        QList<Control*> selectedControls() const;

        bool showOutlines() const;
        void setShowOutlines(bool value);

        bool snapping() const;
        void setSnapping(bool snapping);

        QPointF lastMousePos() const;

    protected:
        virtual bool stick() const;
        virtual QVector<QLineF> guideLines() const;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void drawForeground(QPainter *painter, const QRectF &rect) override;

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
