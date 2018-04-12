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

        friend class ExposerBackend;
        using QGraphicsWidget::contains;

    public:
        bool gui() const;
        bool form() const;
        bool clip() const;
        bool dragIn() const;
        bool dragging() const;
        bool hasErrors() const;
        bool contains(const QString& id) const;

        int higherZValue() const;
        int lowerZValue() const;

        QString id() const;
        QString uid() const;
        QString url() const;
        QString dir() const;

        DesignerScene* scene() const;
        Control* parentControl() const;

        const QList<QString>& events() const;
        const QList<QQmlError>& errors() const;
        const QList<PropertyNode>& properties() const;
        QList<Control*> childControls(bool dive = true) const;

    public:
        static QList<Control*>& controls();

    public:
        void setClip(bool clip);
        void setId(const QString& id);
        void setUrl(const QString& url);
        void setDragIn(bool dragIn);
        void setDragging(bool dragging);
        void setRefreshingDisabled(bool disabled);

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
        virtual void resizeEvent(QGraphicsSceneResizeEvent* event) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR) override;

    protected:
        explicit Control(const QString& url, Control* parent = nullptr);
        ~Control();

    private slots:
        void onSizeChange();
        void onParentChange();
        void onZValueChange();
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
        bool m_refreshingDisabled;

        QString m_url;
        QString m_uid;
        QString m_id;
        QImage m_preview;

        QList<QString> m_events;
        QList<QQmlError> m_errors;
        QList<Resizer*> m_resizers;
        QList<PropertyNode> m_properties;

    private:
        static QList<Control*> m_controls;
};

#endif // CONTROL_H
