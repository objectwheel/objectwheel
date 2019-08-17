#ifndef ANCHORLAYER_H
#define ANCHORLAYER_H

#include <designeritem.h>

class AnchorLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorLayer)

    friend class PaintLayer; // For mousePressPoint()

public:
    explicit AnchorLayer(DesignerItem* parent = nullptr);

    bool activated() const;

public slots:
    void updateGeometry();

private:
    void setActivated(bool activated);
    QPointF mouseLastPoint() const;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseUngrabEvent(QEvent* event) override;

signals:
    void activatedChanged();

private:
    bool m_activated;
    bool m_geometryUpdateScheduled;
    QPointF m_mouseLastPoint;
};


#endif // ANCHORLAYER_H
