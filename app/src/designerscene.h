#ifndef DESIGNERSCENE_H
#define DESIGNERSCENE_H

#include <control.h>
#include <QGraphicsScene>

class DesignerScenePrivate;

class DesignerScene : public QGraphicsScene
{
        Q_OBJECT

    public:
        explicit DesignerScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = Q_NULLPTR);

        const QList<Page*>& pages() const;

        void addPage(Page* page);
        void removePage(Page* page);

        Page* currentPage() const;
        void setCurrentPage(Page* currentPage);

    private:
        DesignerScenePrivate* _d;
        QList<Page*> _pages;
        Page* _currentPage;
};

#endif // DESIGNERSCENE_H
