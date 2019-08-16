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

    bool dragAccepted() const;

    bool raised() const;
    void setRaised(bool raised);

    bool resizable() const;
    void setResizable(bool resizable);

    bool beingDragged() const;
    void setBeingDragged(bool beingDragged);

    bool beingResized() const;
    void setBeingResized(bool beingResized);

    bool beingHighlighted() const;
    void setBeingHighlighted(bool beingHighlighted);

    qreal devicePixelRatio() const;
    qreal width() const;
    qreal height() const;

    QSizeF size() const;
    void setSize(qreal width, qreal height);
    void setSize(const QSizeF& size);

    QRectF boundingRect() const override;
    QRectF rect() const;
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void setRect(const QPointF& topLeft, const QSizeF& size);
    void setRect(const QRectF& rect);

    QRectF geometry() const;
    void setGeometry(qreal x, qreal y, qreal w, qreal h);
    void setGeometry(const QPointF& pos, const QSizeF& size);
    void setGeometry(const QRectF& geometry);

    DesignerScene* scene() const;
    DesignerItem* parentItem() const;
    DesignerItem* topLevelItem() const;
    QList<DesignerItem*> siblingItems() const;
    QList<DesignerItem*> childItems(bool recursive = true) const;

protected:
    QPointF mousePressPoint() const;

protected:
    bool event(QEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    virtual void mouseUngrabEvent(QEvent* event);
    virtual QVariant itemChange(int change, const QVariant& value);

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

signals:
    void doubleClicked();
    void geometryChanged();
    void resizableChanged();
    void beingDraggedChanged();
    void beingResizedChanged();
    void beingHighlightedChanged();

private:
    bool m_raised;
    bool m_resizable;
    bool m_beingDragged;
    bool m_beingResized;
    bool m_beingHighlighted;
    bool m_dragAccepted;
    bool m_inSetGeometry;
    QRectF m_rect;
    QPointF m_mousePressPoint;
    DesignerItem* m_parentItemBeforeRaise;
    QSet<DesignerItem*> m_movableSelectedAncestorItems;
};

#endif // DESIGNERITEM_H
