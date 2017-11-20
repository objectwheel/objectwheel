#ifndef CONTROLVIEW_H
#define CONTROLVIEW_H

#include <QGraphicsView>

class ControlViewPrivate;

class ControlView : public QGraphicsView
{
        Q_OBJECT
    public:
        explicit ControlView(QGraphicsScene* scene, QWidget* parent = 0);

    protected:
        virtual void resizeEvent(QResizeEvent* event) override;
        virtual void contextMenuEvent(QContextMenuEvent *event) override;

    private:
        ControlViewPrivate* _d;
};

#endif // CONTROLVIEW_H
