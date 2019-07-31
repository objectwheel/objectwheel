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
    enum { Type = UserType + 2 };
    int type() const override;

    bool gui() const;
    bool form() const;
    bool popup() const;
    bool window() const;
    bool dragIn() const;
    bool hasErrors() const;
    bool visible() const;

    quint32 index() const;
    QString id() const;
    QString uid() const;
    QString dir() const;
    QImage image() const;
    QRectF frame() const;
    QMarginsF margins() const;

    Control* parentControl() const;

    QVector<QString> events() const;
    QVector<QmlError> errors() const;
    QVector<PropertyNode> properties() const;
    QList<Control*> siblings() const;
    QList<Control*> childControls(bool recursive = true) const;
    QVariant property(const QString& propertyName) const;

public:
    void setId(const QString& id);
    void setDir(const QString& dir);
    void setDragIn(bool dragIn);
    void setIndex(quint32 index);
    void setImage(const QImage& image);
    void setFrame(const QRectF& frame);

protected:
    void dropControl(Control* control);
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void ungrabMouseEvent(QEvent* event) override;
    QVariant itemChange(int change, const QVariant& value) override;

protected:
    void paintImage(QPainter* painter);
    void paintHighlight(QPainter* painter);
    void paintHoverOutline(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    explicit Control(const QString& dir, Control* parent = nullptr);

private slots:
    void applyGeometryCorrection();
    void updateRenderInfo(const RenderResult& result);

private:
    bool m_gui;
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
    QString m_geometryHash;
    QRectF m_geometryCorrection;

    QVector<QString> m_events;
    QVector<QmlError> m_errors;
    QVector<PropertyNode> m_properties;
};

#endif // CONTROL_H
