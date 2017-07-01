#ifndef DESIGNERVIEW_H
#define DESIGNERVIEW_H

#include <QGraphicsView>

class DesignerViewPrivate;

class DesignerView : public QGraphicsView
{
        Q_OBJECT
    public:
        explicit DesignerView(QGraphicsScene* scene, QWidget* parent = 0);

    protected:
        virtual void resizeEvent(QResizeEvent* event) override;
        virtual void contextMenuEvent(QContextMenuEvent *event) override;

    private:
        DesignerViewPrivate* _d;
};

#endif // DESIGNERVIEW_H
