#ifndef CONTROL_H
#define CONTROL_H

#include <QGraphicsWidget>
#include <QUrl>
#include <QList>

class ControlPrivate;

class Control : public QGraphicsWidget
{
    public:
        explicit Control(Control *parent = Q_NULLPTR);
        virtual ~Control();
        QList<Control*> findChildren(const QString& id = QString(), Qt::FindChildOptions option = Qt::FindChildrenRecursively) const;

        QString id() const;
        void setId(const QString& id);

        QUrl url() const;
        void setUrl(const QUrl& url);

        bool showOutline() const;
        void setShowOutline(const bool value);

    protected:
        virtual void refresh();
        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    private:
        QList<Control*> findChildrenRecursively(const QString& id, QList<QGraphicsItem*> parent) const;

    private:
        ControlPrivate* _d;
        QString _id;
        QUrl _url;
        bool _showOutline;
};

class Item : public Control { };

class Page : public Control
{
    private:
        bool _mainPage = false;

    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    public:
        bool mainPage() const;
        void setMainPage(bool mainPage);
};

#endif // CONTROL_H
