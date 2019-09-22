#ifndef MOUSELAYER_H
#define MOUSELAYER_H

#include <designeritem.h>

class Control;
class MouseLayer final : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(MouseLayer)

public:
    explicit MouseLayer(DesignerItem* parent = nullptr);

    bool activated() const;

    QPointF mouseStartPos() const;
    QPointF mouseEndPos() const;

    Control* mouseStartControl() const;
    Control* mouseEndControl() const;

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
    bool m_activated;
    bool m_geometryUpdateScheduled;
    QPointF m_mouseStartPos, m_mouseEndPos;
};

#endif // MOUSELAYER_H
