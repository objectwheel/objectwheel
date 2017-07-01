#ifndef DESIGNERSCENE_H
#define DESIGNERSCENE_H

#include <control.h>
#include <QGraphicsScene>

class DesignerScenePrivate;

class DesignerScene : public QGraphicsScene
{
        Q_OBJECT
        friend class DesignerScenePrivate;

    public:
        enum Skin {
            NoSkin,
            PhonePortrait,
            PhoneLandscape,
            Desktop
        };

        explicit DesignerScene(QObject *parent = Q_NULLPTR);

        const QList<Page*>& pages() const;
        void addPage(Page* page);
        void removePage(Page* page);

        Page* currentPage() const;
        void setCurrentPage(Page* currentPage);

        QList<Control*> controls(Qt::SortOrder order = Qt::DescendingOrder) const;
        QList<Control*> selectedControls() const;

        bool showOutlines() const;
        void setShowOutlines(bool value);

        Skin skin() const;
        void setSkin(const Skin& skin);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        void drawForeground(QPainter *painter, const QRectF &rect) override;

    private:
        DesignerScenePrivate* _d;
        QList<Page*> _pages;
        Page* _currentPage;
        Skin _skin;
};

#endif // DESIGNERSCENE_H
