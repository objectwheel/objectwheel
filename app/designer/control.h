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

public:
    enum { Type = UserType + 2 };
    int type() const override;

    bool gui() const;
    bool form() const;
    bool popup() const;
    bool window() const;
    bool dragIn() const;
    bool visible() const;
    bool hasErrors() const;

    quint32 index() const;
    QString id() const;
    QString uid() const;
    QString dir() const;
    QImage image() const;
    QRectF outerRect() const;
    QMarginsF margins() const;

    QVector<QString> events() const;
    QVector<QmlError> errors() const;
    QVector<PropertyNode> properties() const;
    QVariant property(const QString& propertyName) const;

    Control* parentControl() const;
    QList<Control*> siblings() const;
    QList<Control*> childControls(bool recursive = true) const;

public:
    void setId(const QString& id);
    void setDir(const QString& dir);
    void setDragIn(bool dragIn);
    void setIndex(quint32 index);
    void setImage(const QImage& image);
    void setOuterRect(const QRectF& outerRect);

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
    void paintOutline(QPainter* painter);
    void paintHoverOutline(QPainter* painter, bool hovered);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    explicit Control(const QString& dir, Control* parent = nullptr);

private slots:
    void applyGeometryCorrection();
    void updateRenderInfo(const RenderResult& result);

signals:
    void renderInfoChanged(bool codeChanged);

private:
    bool m_gui;
    bool m_popup;
    bool m_window;
    bool m_dragIn;
    bool m_visible;

    quint32 m_index;
    QString m_dir;
    QString m_id;
    QString m_uid;
    QImage m_image;
    QRectF m_outerRect;
    QSizeF m_snapMargin;
    QMarginsF m_margins;
    QString m_geometryHash;
    QRectF m_geometryCorrection;

    QVector<QString> m_events;
    QVector<QmlError> m_errors;
    QVector<PropertyNode> m_properties;
};

#endif // CONTROL_H
