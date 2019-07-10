#ifndef HEADLINEITEM_H
#define HEADLINEITEM_H

#include <QGraphicsItem>

class Control;
class DesignerScene;

class HeadlineItem final : public QGraphicsItem
{
    Q_DISABLE_COPY(HeadlineItem)

public:
    explicit HeadlineItem(Control* parent);
    DesignerScene* scene() const;
    Control* parentControl() const;
    QRectF boundingRect() const override;

    void setText(const QString& text);
    void updateSize();

private:
    QSizeF textSize() const;
    QString dimensionText() const;
    QFont dimensionTextFont() const;
    qreal dimensionTextWidth() const;
    QFontMetrics dimensionTextFontMetrics() const;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QRectF m_rect;
    QString m_text;
    QPointF m_dragStartPoint;
    bool m_dragStarted;
};

#endif // HEADLINEITEM_H
