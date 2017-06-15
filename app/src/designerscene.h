#ifndef DESIGNERSCENE_H
#define DESIGNERSCENE_H

#include <QGraphicsScene>

class DesignerScene : public QGraphicsScene
{
        Q_OBJECT
    public:
        explicit DesignerScene(QObject *parent = 0);

    protected:
        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;
};

#endif // DESIGNERSCENE_H
