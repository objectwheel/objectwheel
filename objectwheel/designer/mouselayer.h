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

    bool draggingActivated() const;

    QPointF mouseStartPos() const;
    QPointF mouseEndPos() const;

    Control* mouseStartControl() const;
    Control* mouseEndControl() const;

public slots:
    void updateGeometry();

private:
    void setDraggingActivated(bool draggingActivated);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseUngrabEvent(QEvent* event) override;

signals:
    void draggingActivatedChanged();
    void clicked(Control* control, Qt::MouseButtons buttons);

private:
    bool m_draggingActivated;
    bool m_geometryUpdateScheduled;
    QPointF m_mouseStartPos;
    QPointF m_mouseEndPos;
};

#endif // MOUSELAYER_H
