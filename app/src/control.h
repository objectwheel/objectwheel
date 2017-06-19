#ifndef CONTROL_H
#define CONTROL_H

#include <QGraphicsItem>
#include <QUrl>
#include <QList>

class ControlPrivate;

class Control : public QGraphicsItem
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

        static QWidget* puppetWidget();
        static void setPuppetWidget(QWidget* puppetWidget);

        QSizeF size() const;
        void resize(const QSizeF& size);

        QRectF geometry() const;
        void setGeometry(const QRectF& geometry);

    protected:
        virtual QRectF boundingRect() const override;
        virtual void refresh();

        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    private:
        QList<Control*> findChildrenRecursively(const QString& id, QList<QGraphicsItem*> parent) const;

    private:
        ControlPrivate* _d;
        QString _id;
        QUrl _url;
        QSizeF _size;
        bool _showOutline;
        static QPointer<QWidget> _puppetWidget;
};

class Item : public Control { };

class Page : public Control
{
    private:
        bool _main = false;

    public:
        bool isMain() const;
        void setMain(const bool main);
};

#endif // CONTROL_H
