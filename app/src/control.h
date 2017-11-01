#ifndef CONTROL_H
#define CONTROL_H

#include <resizer.h>
#include <qmlpreviewer.h>
#include <QGraphicsWidget>
#include <QList>

class Control;
class ControlPrivate;
class FormPrivate;
class ControlWatcher;

class Control : public QGraphicsWidget
{
        Q_OBJECT
        friend class ControlPrivate;

    public:
        explicit Control(const QString& url, const DesignMode& mode,
          const QString& uid = QString(), Control* parent = nullptr);
        ~Control();
        QString uid() const;
        QString id() const;
        void setId(const QString& id);
        QString url() const;
        void setUrl(const QString& url);
        QString dir() const;
        bool dragging() const;
        bool dragIn() const;
        bool clip() const;
        const PropertyNodes& properties() const;
        const QList<QString>& events() const;
        QList<Control*> childControls(bool dive = true) const;
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
        static void updateUids();
        static QString generateUid();
        static const QList<Control*>& controls();
        const QList<QQmlError>& errors() const;
        bool hasErrors() const;
        const DesignMode& mode() const;
        virtual QRectF frameGeometry() const;
        void setDragging(bool dragging);
        void setDragIn(bool dragIn);

    public slots:
        void hideResizers();
        void showResizers();
        virtual void refresh();
        virtual void centralize();

    protected slots:
        void updateUid();

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
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    signals:
        void previewChanged();
        void errorOccurred();
        void doubleClicked();
        void controlDropped(const QPointF&, const QString&);

    protected:
        bool _clip;
        Resizer _resizers[8];
        ControlPrivate* _d;

    private:
        QString _uid;
        QString _id;
        QList<QString> _events;
        PropertyNodes _properties;
        QList<QQmlError> _errors;
        QString _url;
        DesignMode _mode;
        bool _dragging;
        bool _dragIn;
        bool _gui;
        bool _hideSelection;
        ControlWatcher* _cW;
        static bool _showOutline;
        static QList<Control*> _controls;
};

class Form : public Control
{
        Q_OBJECT
        friend class FormPrivate;

    public:
        explicit Form(const QString& url, const QString& uid = QString(), Form* parent = Q_NULLPTR);
        bool main() const;
        void setMain(bool value);
        void setSkin(const Skin& skin);
        const Skin& skin();
        QRectF frameGeometry() const override;

    signals:
        void skinChanged();

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    private:
        FormPrivate* _d;
        bool _main = false;
        QList<Control*> _controls;
        Skin _skin;
};

#endif // CONTROL_H
