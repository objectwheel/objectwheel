#include <anchorlayer.h>
#include <designerscene.h>
#include <designerview.h>
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

QPointF AnchorLayer::sourceScenePos() const
{
    return m_mousePressPoint;
}

QPointF AnchorLayer::targetScenePos() const
{
    return m_mouseMovePoint;
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

void AnchorLayer::setActivated(bool activated)
{
    if (m_activated != activated) {
        m_activated = activated;
        emit activatedChanged();
    }
}

void AnchorLayer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_mousePressPoint = event->scenePos();
    DesignerItem::mousePressEvent(event);
}

void AnchorLayer::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_ASSERT(scene());

    m_mouseMovePoint = event->scenePos();

    if (scene()->view()->mousePressButton() == Qt::RightButton) {
        if (activated())
            return;

        if (DesignerScene::isInappropriateAnchorSource(sourceControl()))
            return;

        scene()->setCursor(Qt::BlankCursor);
        setActivated(true);
    } else {
        if (activated()) {
            scene()->unsetCursor();
            setActivated(false);
        }
    }

    DesignerItem::hoverMoveEvent(event);
}
