#ifndef ANCHORLAYER_H
#define ANCHORLAYER_H

#include <designeritem.h>

class Control;
class AnchorLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorLayer)

public:
    explicit AnchorLayer(DesignerItem* parent = nullptr);

    bool activated() const;

    QPointF sourceScenePos() const;
    QPointF targetScenePos() const;

    Control* sourceControl() const;
    Control* targetControl() const;

public slots:
    void updateGeometry();

private:
    void setActivated(bool activated);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseUngrabEvent(QEvent* event) override;

signals:
    void activatedChanged();

private:
    bool m_pressed;
    bool m_activated;
    bool m_geometryUpdateScheduled;
    QPointF m_mousePressPoint, m_mouseMovePoint;
};

#endif // ANCHORLAYER_H
