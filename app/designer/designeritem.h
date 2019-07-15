#ifndef DESIGNERITEM_H
#define DESIGNERITEM_H

#include <QGraphicsObject>
#include <QSet>

class DesignerScene;
class DesignerItem : public QGraphicsObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerItem)

    friend class HeadlineItem; // For setBeingDragged
    friend class ResizerItem; // For setBeingResized and setBeingDragged

public:
    enum DesignerItemChange {
        ItemSizeChange = 100,
        ItemSizeHasChanged
    };

public:
    explicit DesignerItem(DesignerItem* parent = nullptr);

    enum { Type = UserType + 1 };
    int type() const override;

    bool beingDragged() const;
    bool beingResized() const;

    DesignerScene* scene() const;
    DesignerItem* parentItem() const;

    QList<DesignerItem*> siblingItems() const;
    QList<DesignerItem*> childItems(bool recursive = true) const;

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

protected:
    bool dragAccepted() const;
    QPointF mousePressPoint() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual QVariant itemChange(int change, const QVariant& value);

private:
    void setBeingDragged(bool beingDragged);
    void setBeingResized(bool beingResized);

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

signals:
    void doubleClicked();
    void geometryChanged();
    void beingDraggedChanged();
    void beingResizedChanged();

private:
    bool m_inSetGeometry;
    bool m_beingDragged;
    bool m_beingResized;
    bool m_dragAccepted;
    QRectF m_rect;
    QPointF m_mousePressPoint;
    QSet<DesignerItem*> m_movableSelectedAncestorItems;
};

#endif // DESIGNERITEM_H
