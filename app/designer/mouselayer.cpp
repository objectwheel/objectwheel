#include <mouselayer.h>
#include <designerscene.h>
#include <designerview.h>
#include <QGraphicsSceneMouseEvent>

MouseLayer::MouseLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_activated(false)
  , m_geometryUpdateScheduled(false)
{
}

bool MouseLayer::activated() const
{
    return m_activated;
}

QPointF MouseLayer::mouseStartPos() const
{
    return m_mouseStartPos;
}

QPointF MouseLayer::mouseEndPos() const
{
    return m_mouseEndPos;
}

Control* MouseLayer::mouseStartControl() const
{
    Q_ASSERT(scene());
    return scene()->topLevelControl(mouseStartPos());
}

Control* MouseLayer::mouseEndControl() const
{
    Q_ASSERT(scene());
    return scene()->topLevelControl(mouseEndPos());
}

void MouseLayer::updateGeometry()
{
    if (m_geometryUpdateScheduled)
        return;
    m_geometryUpdateScheduled = true;
    QMetaObject::invokeMethod(this, [=] {
        Q_ASSERT(scene());
        setGeometry(QPointF(), scene()->sceneRect().size());
        m_geometryUpdateScheduled = false;
    }, Qt::QueuedConnection);
}

void MouseLayer::setActivated(bool activated)
{
    if (m_activated != activated) {
        m_activated = activated;
        emit activatedChanged();
    }
}

void MouseLayer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mousePressEvent(event);

    m_mouseStartPos = event->scenePos();

    if (DesignerScene::isInappropriateAnchorSource(mouseStartControl()))
        return;

    if (event->button() == Qt::RightButton)
        event->accept();
}

void MouseLayer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mouseMoveEvent(event);

    if (dragAccepted() && !activated()) {
        scene()->setCursor(Qt::BlankCursor);
        setActivated(true);
    }

    m_mouseEndPos = event->scenePos();
    update();
}

void MouseLayer::mouseUngrabEvent(QEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mouseUngrabEvent(event); // Clears dragAccepted state

    if (activated()) {
        scene()->unsetCursor();
        setActivated(false);
    }
}
