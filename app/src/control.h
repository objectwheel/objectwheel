#ifndef CONTROL_H
#define CONTROL_H

#include <QGraphicsWidget>
#include <QUrl>
#include <QList>
#include <QPixmap>
#include <QTimer>
#include <controltransaction.h>

#define MAX_Z_VALUE (9999999)

class Control;
class ControlPrivate;
class PagePrivate;

class Resizer : public QGraphicsWidget
{
        Q_OBJECT

    public:
        enum Placement {
            Top,
            Right,
            Bottom,
            Left,
            TopLeft,
            TopRight,
            BottomRight,
            BottomLeft
        };

        explicit Resizer(Control* parent = Q_NULLPTR);

        Placement placement() const;
        void setPlacement(const Placement& placement);

        bool disabled() const;
        void setDisabled(bool disabled);

        static bool resizing();

    protected:
        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    private:
        Placement _placement;
        bool _disabled;
        static bool _resizing;
};

class Control : public QGraphicsWidget
{
        Q_OBJECT
        friend class ControlPrivate;

    public:
        explicit Control(const QUrl& url, Control* parent = Q_NULLPTR);

        QString id() const;
        void setId(const QString& id);

        QUrl url() const;

        static bool showOutline();
        static void setShowOutline(const bool value);

        QList<Control*> childControls() const;
        Control* parentControl() const;

        QList<Control*> collidingControls(Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;

        bool dragging() const;
        void setDragging(bool dragging);

        bool dragIn() const;
        void setDragIn(bool dragIn);

        bool clip() const;
        void setClip(bool clip);

        QMap<QString, QVariant::Type> properties() const;
        void setProperties(const QMap<QString, QVariant::Type>& properties);

        QList<QString> events() const;
        void setEvents(const QList<QString>& events);

    public slots:
        virtual void refresh();

    protected:
        virtual void dropControl(Control* control);
        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    signals:
        void idChanged(const QString&);
        void previewChanged();
        void initialized();

    protected:
        Resizer _resizers[8];
        ControlPrivate* _d;

    private:
        ControlTransaction _controlTransaction;
        QString _id;
        QList<QString> _events;
        QMap<QString, QVariant::Type> _properties;
        const QUrl _url;
        bool _dragging;
        bool _dragIn;
        bool _clip;
        static bool _showOutline;
};

class Page : public Control
{
        Q_OBJECT
        friend class PagePrivate;

    public:
        enum Skin {
            NoSkin,
            PhonePortrait,
            PhoneLandscape,
            Desktop
        };

        explicit Page(const QUrl& url, Page* parent = Q_NULLPTR);

        bool isMain() const;
        void setMain(bool value);

        static void setSkin(const Skin& skin);
        static const Skin& skin();

        bool stickSelectedControlToGuideLines() const;
        QVector<QLineF> guideLines() const;

        using Control::contains;
        bool contains(const QString& id) const;

        QRectF frameGeometry() const;

        int higherZValue() const;
        int lowerZValue() const;

    public slots:
        void centralize();
        void cleanPage();

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    private:
        PagePrivate* _d;
        bool _main = false;
        QList<Control*> _controls;
        static Skin _skin;
};

#endif // CONTROL_H
