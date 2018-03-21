#ifndef CONTROL_H
#define CONTROL_H

#include <previewresult.h>
#include <QGraphicsWidget>

class Resizer;
class ControlWatcher;
class DesignerScene;

class Control : public QGraphicsWidget
{
        Q_OBJECT

    public:
        explicit Control(const QString& url, const QString& uid = QString(), Control* parent = nullptr);
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
        const QList<PropertyNode>& properties() const;
        const QList<QString>& events() const;
        QList<Control*> childControls(bool dive = true) const;
        Control* parentControl() const;
        int higherZValue() const;
        int lowerZValue() const;
        bool gui() const;
        using QGraphicsWidget::contains;
        bool contains(const QString& id) const;
        bool form() const;
        static bool showOutline();
        static void setShowOutline(const bool value);
        static void updateUids();
        static QString generateUid();
        static QList<Control*>& controls();
        const QList<QQmlError>& errors() const;
        bool hasErrors() const;
        virtual QRectF frameGeometry() const;
        void setDragging(bool dragging);
        void setDragIn(bool dragIn);
        DesignerScene* scene() const;

    public slots:
        void hideResizers();
        void showResizers();
        virtual void refresh(bool repreview = false);
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

    private slots:
        void updateAnchors(const Anchors& anchors);
        void updatePreview(const PreviewResult& result);

    private:
        QImage initialPreview() const;

    signals:
        void previewChanged();
        void errorOccurred();
        void doubleClicked();
        void controlDropped(const QPointF&, const QString&);

    protected:
        bool m_clip;
        QList<Resizer*> m_resizers;

    private:
        QImage m_preview;
        QString m_uid;
        QString m_id;
        QList<QString> m_events;
        QList<PropertyNode> m_properties;
        QList<QQmlError> m_errors;
        QString m_url;
        bool m_hoverOn;
        bool m_dragging;
        bool m_dragIn;
        bool m_gui;
        bool m_hideSelection;
        ControlWatcher* m_cW;
        static bool m_showOutline;
        static QList<Control*> m_controls;
};

class Form : public Control
{
        Q_OBJECT

    public:
        explicit Form(const QString& url, const QString& uid = QString(), Form* parent = nullptr);
        QRectF frameGeometry() const override;
        void setMain(bool value);
        bool main() const;

    protected:
        void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        bool m_main = false;
        QList<Control*> m_controls;
};

#endif // CONTROL_H
