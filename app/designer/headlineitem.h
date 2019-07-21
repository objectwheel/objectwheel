#ifndef HEADLINEITEM_H
#define HEADLINEITEM_H

#include <gadgetitem.h>

class HeadlineItem final : public GadgetItem
{
    Q_OBJECT
    Q_DISABLE_COPY(HeadlineItem)

public:
    explicit HeadlineItem(DesignerItem* parent = nullptr);

    QSizeF dimensions() const;
    void setDimensions(const QSizeF& dimensions);

    QString text() const;
    void setText(const QString& text);

    void scheduleSizeUpdate();

private:
    void updateSize();
    QFont dimensionsFont() const;
    QString dimensionsText(qreal width, qreal height) const;
    QSizeF calculateTextSize() const;

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_text;
    QSizeF m_dimensions;
    bool m_sizeUpdateScheduled;
};

#endif // HEADLINEITEM_H
