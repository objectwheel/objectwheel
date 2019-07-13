#ifndef DESIGNERITEM_H
#define DESIGNERITEM_H

#include <QGraphicsObject>
#include <QPen>
#include <QFont>
#include <QSet>

class DesignerScene;
class DesignerItem : public QGraphicsObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerItem)

    friend class HeadlineItem; // For setBeingDragged

public:
    explicit DesignerItem(DesignerItem* parent = nullptr);

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

protected:
    QPointF dragDistance() const;
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
    bool m_inSetGeometry;
    bool m_beingDragged;
    bool m_dragDistanceExceeded;
    QPen m_pen;
    QBrush m_brush;
    QFont m_font;
    QRectF m_rect;
    QPointF m_dragDistance;
    QPointF m_dragStartPoint;
    QSet<DesignerItem*> m_movableSelectedAncestorItems;
};

#endif // DESIGNERITEM_H
