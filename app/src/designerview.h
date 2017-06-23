#ifndef DESIGNERVIEW_H
#define DESIGNERVIEW_H

#include <QGraphicsView>

class DesignerView : public QGraphicsView
{
        Q_OBJECT
    public:
        explicit DesignerView(QGraphicsScene* scene, QWidget* parent = 0);

    protected:
        virtual void resizeEvent(QResizeEvent* event) override;
};

#endif // DESIGNERVIEW_H
