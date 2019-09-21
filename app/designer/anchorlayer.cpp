#include <anchorlayer.h>
#include <designerscene.h>
#include <QGraphicsSceneMouseEvent>

AnchorLayer::AnchorLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_pressed(false)
  , m_activated(false)
  , m_geometryUpdateScheduled(false)
{
}

bool AnchorLayer::activated() const
{
    return m_activated;
}

QPointF AnchorLayer::sourceScenePos() const
{
    return mapToScene(m_mousePressPoint);
}

QPointF AnchorLayer::targetScenePos() const
{
    return mapToScene(m_mouseMovePoint);
}

Control* AnchorLayer::sourceControl() const
{
    Q_ASSERT(scene());
    return scene()->topLevelControl(sourceScenePos());
}

Control* AnchorLayer::targetControl() const
{
    Q_ASSERT(scene());
    return scene()->topLevelControl(targetScenePos());
}

void AnchorLayer::setActivated(bool activated)
{
    if (m_activated != activated) {
        m_activated = activated;
        emit activatedChanged();
    }
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

    if (DesignerScene::isInappropriateAnchorSource(scene()->topLevelControl(event->scenePos())))
        return;

    if (event->button() == Qt::RightButton) {
        m_pressed = true;
        m_mousePressPoint = event->pos();
        event->accept();
    }
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
