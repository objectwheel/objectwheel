#ifndef WINDOWVIEW_H
#define WINDOWVIEW_H

#include <QGraphicsView>

class WindowViewPrivate;

class WindowView : public QGraphicsView
{
        Q_OBJECT
    public:
        explicit WindowView(QGraphicsScene* scene, QWidget* parent = 0);

    protected:
        virtual void resizeEvent(QResizeEvent* event) override;
        virtual void contextMenuEvent(QContextMenuEvent *event) override;

    private:
        WindowViewPrivate* _d;
};

#endif // WINDOWVIEW_H
