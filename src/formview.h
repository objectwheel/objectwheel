#ifndef FORMVIEW_H
#define FORMVIEW_H

#include <QGraphicsView>

class FormViewPrivate;

class FormView : public QGraphicsView
{
        Q_OBJECT
    public:
        explicit FormView(QGraphicsScene* scene, QWidget* parent = 0);

    protected:
        virtual void resizeEvent(QResizeEvent* event) override;
        virtual void contextMenuEvent(QContextMenuEvent *event) override;

    private:
        FormViewPrivate* _d;
};

#endif // FORMVIEW_H
