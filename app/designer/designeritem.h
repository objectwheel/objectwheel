#ifndef DESIGNERITEM_H
#define DESIGNERITEM_H

#include <QGraphicsObject>
#include <QPen>
#include <QFont>

class Control;
class DesignerScene;

class DesignerItem : public QGraphicsObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerItem)

public:
    explicit DesignerItem(Control* parent = nullptr);

    DesignerScene* scene() const;
    Control* parentControl() const;

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

protected:
    bool dragStarted() const;
    QPointF snapPosition() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void doubleClicked();
    void dragStartedChanged();

private:
    QRectF m_rect;
    QFont m_font;
    QPen m_pen;
    QBrush m_brush;
    bool m_dragStarted;
    QPointF m_snapPosition;
    QPointF m_dragStartPoint;
};

#endif // DESIGNERITEM_H
