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

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        virtual void refresh();

    private:
        QList<Control*> findChildrenRecursively(const QString& id, QList<QGraphicsItem*> parent) const;

    private:
        ControlPrivate* _d;
        QString _id;
        QUrl _url;
};

class Item : Control { };

class Page : Control
{
    private:
        bool _main = false;

    public:
        bool isMain() const;
        void setMain(const bool main);
};

#endif // CONTROL_H
