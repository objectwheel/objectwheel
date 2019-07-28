#ifndef HEADLINEITEM_H
#define HEADLINEITEM_H

#include <gadgetitem.h>
#include <QPointer>

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

    DesignerItem* targetItem() const;
    void setTargetItem(DesignerItem* targetItem);

public slots:
    void updateGeometry();

private:
    QFont dimensionsFont() const;
    QString dimensionsText(qreal width, qreal height) const;
    QSizeF calculateTextSize() const;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void ungrabMouseEvent(QEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_text;
    QSizeF m_dimensions;
    QPointer<DesignerItem> m_targetItem;
    bool m_geometryUpdateScheduled;
};

#endif // HEADLINEITEM_H
