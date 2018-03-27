#ifndef CONTROL_H
#define CONTROL_H

#include <previewresult.h>
#include <QGraphicsWidget>

class Resizer;
class DesignerScene;
class ControlWatcher;

class Control : public QGraphicsWidget
{
        Q_OBJECT
        using QGraphicsWidget::contains;

    public:
        explicit Control(const QString& url, Control* parent = nullptr);
        ~Control();

        bool gui() const;
        bool form() const;
        bool clip() const;
        bool dragIn() const;
        bool dragging() const;
        bool hasErrors() const;
        bool contains(const QString& id) const;

        int higherZValue() const;
        int lowerZValue() const;

        QString uid() const;
        QString id() const;
        QString url() const;
        QString dir() const;

        const QList<PropertyNode>& properties() const;
        const QList<QQmlError>& errors() const;
        const QList<QString>& events() const;
        QList<Control*> childControls(bool dive = true) const;

        virtual QRectF frameGeometry() const;

        DesignerScene* scene() const;
        Control* parentControl() const;

    public:
        static QList<Control*>& controls();

    public:
        void setId(const QString& id);
        void setUrl(const QString& url);
        void setDragIn(bool dragIn);
        void setDragging(bool dragging);

    public slots:
        void updateUid();
        void centralize();
        void hideResizers();
        void showResizers();
        void refresh(bool repreview = false);

    public slots:
        static void updateUids();

    private:
        void dropControl(Control* control);
        void dropEvent(QGraphicsSceneDragDropEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
        void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
        void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
        QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void resizeEvent(QGraphicsSceneResizeEvent* event) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR) override;

    private slots:
        void updateAnchors(const Anchors& anchors);
        void updatePreview(const PreviewResult& result);

    signals:
        void doubleClicked();
        void errorOccurred();
        void previewChanged();
        void controlDropped(const QPointF&, const QString&);

    private:
        bool m_gui;
        bool m_clip;
        bool m_dragIn;
        bool m_hoverOn;
        bool m_dragging;

        QString m_uid;
        QString m_id;
        QString m_url;
        QImage m_preview;

        QList<QString> m_events;
        QList<QQmlError> m_errors;
        QList<Resizer*> m_resizers;
        QList<PropertyNode> m_properties;

    private:
        static QList<Control*> m_controls;
};

class Form : public Control
{
        Q_OBJECT

    public:
        explicit Form(const QString& url, Form* parent = nullptr);
        QRectF frameGeometry() const override;
        void setMain(bool value);
        bool main() const;

    protected:
        void resizeEvent(QGraphicsSceneResizeEvent* event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        bool m_main = false;
};

#endif // CONTROL_H
