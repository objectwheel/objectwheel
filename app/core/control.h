#ifndef CONTROL_H
#define CONTROL_H

#include <previewresult.h>
#include <QGraphicsWidget>

class Resizer;
class DesignerScene;

class Control : public QGraphicsWidget
{
    Q_OBJECT

    friend class ControlCreationManager; // For constructor
    friend class ProjectExposingManager; // For constructor
    friend class DesignerScene; // For destructor (delete operator)
    using QGraphicsWidget::contains;

public:
    bool gui() const;
    bool form() const;
    bool clip() const;
    bool window() const;
    bool dragIn() const;
    bool dragging() const;
    bool resizing() const;
    bool hasErrors() const;
    bool contains(const QString& id) const;

    int higherZValue() const;
    int lowerZValue() const;

    QString id() const;
    QString uid() const;
    QString url() const;
    QString dir() const;
    QMarginsF margins() const;

    DesignerScene* scene() const;
    Control* parentControl() const;

    QList<QQmlError> errors() const;
    QList<QString> events() const;
    QList<PropertyNode> properties() const;
    QList<Control*> childControls(bool dive = true) const;
    QVariant::Type propertyType(const QString& propertyName) const;

    static QList<Control*> controls();

public:
    void setClip(bool clip);
    void setId(const QString& id);
    void setUrl(const QString& url);
    void setDragIn(bool dragIn);
    void setDragging(bool dragging);
    void setResizing(bool resizing);

public slots:
    void hideResizers();
    void showResizers();

signals:
    void resizingChanged();
    void draggingChanged();

private:
    void dropControl(Control* control);
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

protected:
    void resizeEvent(QGraphicsSceneResizeEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR) override;

protected:
    explicit Control(const QString& url, Control* parent = nullptr);
    ~Control() override;

private slots:
    void updatePreview(const PreviewResult& result);
    void applyCachedGeometry();

private:
    bool m_gui;
    bool m_clip;
    bool m_window;
    bool m_dragIn;
    bool m_hoverOn;
    bool m_dragging;
    bool m_resizing;
    QMarginsF m_margins;
    QRectF m_cachedGeometry;

    QString m_url;
    QString m_uid;
    QString m_id;
    QImage m_image;

    QList<QQmlError> m_errors;
    QList<Resizer*> m_resizers;
    QList<QString> m_events;
    QList<PropertyNode> m_properties;

private:
    static QList<Control*> m_controls;
};

#endif // CONTROL_H
