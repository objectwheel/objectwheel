#include <anchorlayer.h>
#include <designerscene.h>
#include <QGraphicsSceneMouseEvent>

AnchorLayer::AnchorLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_activated(false)
  , m_geometryUpdateScheduled(false)
{
}

bool AnchorLayer::activated() const
{
    return m_activated;
}

void AnchorLayer::setActivated(bool activated)
{
    if (m_activated != activated) {
        m_activated = activated;
        emit activatedChanged();
    }
}

QPointF AnchorLayer::mouseMovePoint() const
{
    return m_mouseMovePoint;
}

void AnchorLayer::updateGeometry()
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

void AnchorLayer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mousePressEvent(event);

    if (DesignerScene::isInappropriateAnchorSource(scene()->topLevelControl(mapToScene(mousePressPoint()))))
        return;

    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)
        event->accept();
}

void AnchorLayer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mouseMoveEvent(event);

    if (dragAccepted() && !activated()) {
        scene()->setCursor(Qt::BlankCursor);
        setActivated(true);
    }

    m_mouseMovePoint = event->pos();
    update();
}

void AnchorLayer::mouseUngrabEvent(QEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mouseUngrabEvent(event); // Clears dragAccepted state

    if (activated()) {
        scene()->unsetCursor();
        setActivated(false);
    }
}
