#ifndef SCENEEXTENDITEM_H
#define SCENEEXTENDITEM_H

#include <QGraphicsItem>

class Control;
class SceneExtendItem final : public QGraphicsItem
{
    Q_DISABLE_COPY(SceneExtendItem)

public:
    explicit SceneExtendItem(Control* parent);
    Control* parentControl() const;
    QRectF boundingRect() const override;
    void updateRect();

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

private:
    QRectF m_rect;
};

#endif // SCENEEXTENDITEM_H
