#ifndef DESIGNERSCENE_H
#define DESIGNERSCENE_H

#include <QGraphicsScene>

class DesignerScene : public QGraphicsScene
{
        Q_OBJECT

    public:
        explicit DesignerScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = Q_NULLPTR);
};

#endif // DESIGNERSCENE_H
