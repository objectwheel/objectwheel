#ifndef DESIGNERITEM_H
#define DESIGNERITEM_H

#include <QGraphicsObject>
#include <QPen>
#include <QFont>
#include <QSet>

class Control;
class Form;
class DesignerScene;

class DesignerItem : public QGraphicsObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerItem)

public:
    explicit DesignerItem(DesignerItem* parent = nullptr);

    virtual Control* controlCast();
    virtual Form* formCast();

    DesignerScene* scene() const;
    DesignerItem* parentItem() const;

    QList<DesignerItem*> siblingItems() const;
    QList<DesignerItem*> childItems(bool recursive = true) const;

    QPen pen() const;
    void setPen(const QPen& pen);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

    QFont font() const;
    void setFont(const QFont& font);

    qreal width() const;
    qreal height() const;

    QSizeF size() const;
    void setSize(const QSizeF& size);
    void setSize(qreal width, qreal height);

    QRectF rect() const;
    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);

    QRectF geometry() const;
    void setGeometry(const QRectF& geometry);
    void setGeometry(qreal x, qreal y, qreal w, qreal h);

    QRectF boundingRect() const override;

    enum { Type = UserType + 1 };
    int type() const override;

    bool beingDragged() const;
    QPointF dragDistance() const;

protected:
    bool dragDistanceExceeded() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void setBeingDragged(bool beingDragged);
    void setDragDistance(const QPointF& dragDistance);

signals:
    void doubleClicked();
    void geometryChanged();
    void beingDraggedChanged();

private:
    QRectF m_rect;
    QFont m_font;
    QPen m_pen;
    QBrush m_brush;
    bool m_inSetGeometry;
    bool m_beingDragged;
    QPointF m_dragDistance;
    QSet<DesignerItem*> m_movableSelectedAncestorItems;
    QPointF m_dragStartPoint;
    bool m_dragDistanceExceeded;
};

#endif // DESIGNERITEM_H
