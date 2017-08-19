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
class FormPrivate;

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
        friend class ControlScene;
        friend class ControlView;
        friend class FormScene;
        friend class DesignManager;
        friend class DesignManagerPrivate;

    public:
        explicit Control(const QUrl& url, Control* parent = Q_NULLPTR);
        QString uid() const;
        QString id() const;
        void setId(const QString& id);
        QUrl url() const;
        bool dragging() const;
        bool dragIn() const;
        bool clip() const;
        QMap<QString, QVariant::Type> properties() const;
        QList<QString> events() const;
        QList<Control*> childControls() const;
        Control* parentControl() const;
        int higherZValue() const;
        int lowerZValue() const;
        bool stickSelectedControlToGuideLines() const;
        QVector<QLineF> guideLines() const;
        bool gui() const;
        using QGraphicsWidget::contains;
        bool contains(const QString& id) const;
        bool form() const;
        static bool showOutline();
        static void setShowOutline(const bool value);
        QString dir() const;
        void setDir(const QString& dir);

    public slots:
        void hideSelection();
        void showSelection();
        void hideResizers();
        void showResizers();
        void cleanContent();
        virtual void refresh();

    protected:
        void setGui(bool value);
        void setDragging(bool dragging);
        void setDragIn(bool dragIn);
        void setClip(bool clip);
        void setProperties(const QMap<QString, QVariant::Type>& properties);
        void setEvents(const QList<QString>& events);

        virtual void centralize();
        virtual QRectF frameGeometry() const;
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
        const QString _uid;
        QString _id;
        QList<QString> _events;
        QMap<QString, QVariant::Type> _properties;
        const QUrl _url;
        QString _dir;
        bool _dragging;
        bool _dragIn;
        bool _clip;
        bool _gui;
        bool _hideSelection;
        static bool _showOutline;
};

class Form : public Control
{
        Q_OBJECT
        friend class FormPrivate;

    public:
        enum Skin {
            NoSkin,
            PhonePortrait,
            PhoneLandscape,
            Desktop
        };

        explicit Form(const QUrl& url, Form* parent = Q_NULLPTR);

        bool isMain() const;
        void setMain(bool value);

        QRectF frameGeometry() const override;

        static void setSkin(const Skin& skin);
        static const Skin& skin();

    public slots:
        void centralize() override;

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    private:
        FormPrivate* _d;
        bool _main = false;
        QList<Control*> _controls;
        static Skin _skin;
};

#endif // CONTROL_H
