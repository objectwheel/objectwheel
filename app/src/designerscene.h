#ifndef DESIGNERSCENE_H
#define DESIGNERSCENE_H

#include <control.h>
#include <QGraphicsScene>

class DesignerScenePrivate;

class DesignerScene : public QGraphicsScene
{
        Q_OBJECT
        Q_DISABLE_COPY(DesignerScene)
        friend class DesignerScenePrivate;

    public:
        explicit DesignerScene(QObject *parent = Q_NULLPTR);
        static DesignerScene* instance();

        static const QList<Page*>& pages();
        static void addPage(Page* page);
        static void removePage(Page* page);

        static Page* currentPage();
        static void setCurrentPage(Page* currentPage);

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
        static DesignerScenePrivate* _d;
        static QList<Page*> _pages;
        static Page* _currentPage;
        static bool _snapping;
        static QPointF _lastMousePos;

};

#endif // DESIGNERSCENE_H
