#include <mouselayer.h>
#include <designerscene.h>
#include <designerview.h>
#include <QGraphicsSceneMouseEvent>

MouseLayer::MouseLayer(DesignerItem* parent) : DesignerItem(parent)
  , m_draggingActivated(false)
  , m_geometryUpdateScheduled(false)
{
    setMousePressCursorShape(Qt::BlankCursor);
}

bool MouseLayer::draggingActivated() const
{
    return m_draggingActivated;
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

void MouseLayer::setDraggingActivated(bool draggingActivated)
{
    if (m_draggingActivated != draggingActivated) {
        m_draggingActivated = draggingActivated;
        emit draggingActivatedChanged();
    }
}

void MouseLayer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mousePressEvent(event);

    m_mouseStartPos = event->scenePos();
    m_mouseEndPos = m_mouseStartPos;

    if (DesignerScene::isInappropriateAnchorSource(mouseStartControl()))
        return;

    if (event->modifiers() == Qt::NoModifier)
        event->accept();
}

void MouseLayer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mouseMoveEvent(event);

    m_mouseEndPos = event->scenePos();

    if (dragAccepted() && !draggingActivated())
        setDraggingActivated(true);

    update();
}

void MouseLayer::mouseUngrabEvent(QEvent* event)
{
    Q_ASSERT(scene());

    DesignerItem::mouseUngrabEvent(event); // Clears dragAccepted state

    if (draggingActivated())
        setDraggingActivated(false);
    else
        emit clicked(mouseEndControl(), scene()->view()->mousePressButton());
}
