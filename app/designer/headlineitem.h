#ifndef HEADLINEITEM_H
#define HEADLINEITEM_H

#include <designeritem.h>

class HeadlineItem final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(HeadlineItem)

public:
    explicit HeadlineItem(Control* parent);
    void setText(const QString& text);
    bool showDimensions() const;
    void setShowDimensions(bool showDimensions);
    void updateSize();

private:
    QSizeF calculateTextSize() const;
    QFont dimensionTextFont() const;
    QString dimensionText(qreal width, qreal height) const;

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_text;
    bool m_showDimensions;
};

#endif // HEADLINEITEM_H
