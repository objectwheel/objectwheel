#ifndef HEADLINEITEM_H
#define HEADLINEITEM_H

#include <designeritem.h>

class HeadlineItem final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(HeadlineItem)

public:
    explicit HeadlineItem(Control* parent);
    QRectF boundingRect() const override;
    void setText(const QString& text);
    void updateSize();

private:
    QSizeF textSize() const;
    QFont dimensionTextFont() const;
    QString dimensionText(int width, int height) const;

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QRectF m_rect;
    QString m_text;
};

#endif // HEADLINEITEM_H
