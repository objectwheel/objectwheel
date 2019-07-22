#ifndef CONTROL_H
#define CONTROL_H

#include <renderresult.h>
#include <designeritem.h>

class DesignerScene;

class Control : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(Control)

    friend class ControlCreationManager; // For constructor
    friend class ProjectExposingManager; // For constructor
    friend class DesignerScene; // For destructor (delete operator)

public:
    bool gui() const;
    bool form() const;
    bool clip() const;
    bool popup() const;
    bool window() const;
    bool dragIn() const;
    bool hasErrors() const;
    bool visible() const;

    quint32 index() const;
    int higherZValue() const;
    int lowerZValue() const;

    enum { Type = UserType + 2 };
    int type() const override;

    QString id() const;
    QString uid() const;
    QString dir() const;
    QMarginsF margins() const;
    QImage image() const;

    Control* parentControl() const;

    QVector<QmlError> errors() const;
    QVector<QString> events() const;
    QVector<PropertyNode> properties() const;
    QList<Control*> siblings() const;
    QList<Control*> childControls(bool recursive = true) const;
    QVariant::Type propertyType(const QString& propertyName) const;

    static QVector<Control*> controls();

public:
    void setClip(bool clip);
    void setId(const QString& id);
    void setDir(const QString& dir);
    void setDragIn(bool dragIn);
    void setIndex(quint32 index);

    QRectF frame() const;

protected:
    void dropControl(Control* control);
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(int change, const QVariant& value) override;

protected:
    virtual void restrainPaintRegion(QPainter* painter);
    virtual void paintImage(QPainter* painter);
    virtual void paintHighlight(QPainter* painter);
    virtual void paintHoverOutline(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    explicit Control(const QString& dir, Control* parent = nullptr);
    ~Control() override;

private slots:
    void updateGeometry();
    void updateImage(const RenderResult& result);

private:
    bool m_gui;
    bool m_clip;
    bool m_popup;
    bool m_window;
    bool m_dragIn;
    bool m_visible;

    quint32 m_index;
    QString m_dir;
    QString m_uid;
    QString m_id;
    QRectF m_frame;
    QImage m_image;
    QSizeF m_snapMargin;
    QMarginsF m_margins;
    QRectF m_cachedGeometry;
    QVector<QString> m_blockedPropertyChanges;

    QVector<QmlError> m_errors;
    QVector<QString> m_events;
    QVector<PropertyNode> m_properties;

private:
    static QVector<Control*> m_controls;
};

#endif // CONTROL_H
