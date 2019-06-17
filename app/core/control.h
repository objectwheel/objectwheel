#ifndef CONTROL_H
#define CONTROL_H

#include <previewresult.h>
#include <QGraphicsWidget>

class Resizer;
class DesignerScene;

class Control : public QGraphicsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(Control)

    friend class ControlCreationManager; // For constructor
    friend class ProjectExposingManager; // For constructor
    friend class DesignerScene; // For destructor (delete operator)

public:
    enum { Type = UserType + 1 };

public:
    bool gui() const;
    bool form() const;
    bool clip() const;
    bool popup() const;
    bool window() const;
    bool dragIn() const;
    bool dragging() const;
    bool resized() const;
    bool hasErrors() const;

    quint32 index() const;
    int type() const override;
    int higherZValue() const;
    int lowerZValue() const;

    QString id() const;
    QString uid() const;
    QString dir() const;
    QMarginsF margins() const;

    DesignerScene* scene() const;
    Control* parentControl() const;

    QList<QmlError> errors() const;
    QList<QString> events() const;
    QList<PropertyNode> properties() const;
    QList<Control*> siblings() const;
    QList<Control*> childControls(bool recursive = true) const;
    QVariant::Type propertyType(const QString& propertyName) const;

    static QList<Control*> controls();

public:
    void setClip(bool clip);
    void setId(const QString& id);
    void setDir(const QString& dir);
    void setDragIn(bool dragIn);
    void setDragging(bool dragging);
    void setResized(bool resized);
    void setIndex(quint32 index);

public slots:
    void hideResizers();
    void showResizers();

signals:
    void resizedChanged();
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
    explicit Control(const QString& dir, Control* parent = nullptr);
    ~Control() override;

private slots:
    void updatePreview(const PreviewResult& result);
    void applyCachedGeometry();

private:
    bool m_gui;
    bool m_clip;
    bool m_popup;
    bool m_window;
    bool m_dragIn;
    bool m_hoverOn;
    bool m_dragging;
    bool m_resized;

    quint32 m_index;
    QMarginsF m_margins;
    QRectF m_cachedGeometry;

    QString m_dir;
    QString m_uid;
    QString m_id;
    QPixmap m_pixmap;

    QList<QmlError> m_errors;
    QList<Resizer*> m_resizers;
    QList<QString> m_events;
    QList<PropertyNode> m_properties;

private:
    static QList<Control*> m_controls;
};

#endif // CONTROL_H
