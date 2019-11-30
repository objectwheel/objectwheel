#include <designerview.h>
#include <designerscene.h>
#include <private/qgraphicsview_p.h>

#include <QMouseEvent>
#include <QScrollBar>
#include <QCoreApplication>
#include <QPainter>

DesignerView::DesignerView(QWidget* parent) : QGraphicsView(new DesignerScene(parent), parent)
  , m_panningState(Panning::NotStarted)
{
    setDragMode(RubberBandDrag);
    setResizeAnchor(AnchorViewCenter);
    setTransformationAnchor(AnchorUnderMouse);
    setViewportUpdateMode(FullViewportUpdate);
    setAlignment(Qt::AlignCenter);
    setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setBackgroundRole(QPalette::Window);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false); // Workaround for QTBUG-46351

    // as mousetracking only works for mouse key it is better to handle it in the
    // eventFilter method so it works also for the space scrolling case as expected
    QCoreApplication::instance()->installEventFilter(this);
}

DesignerScene* DesignerView::scene() const
{
    return static_cast<DesignerScene*>(QGraphicsView::scene());
}

Qt::MouseButton DesignerView::mousePressButton() const
{
    Q_D(const QGraphicsView);
    return d->mousePressButton;
}

void DesignerView::startPanning(QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
        m_panningState = Panning::SpaceKeyStarted;
    else
        m_panningState = Panning::MouseWheelStarted;
    viewport()->setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void DesignerView::stopPanning(QEvent* event)
{
    m_panningState = Panning::NotStarted;
    m_panningStartPosition = QPoint();
    viewport()->unsetCursor();
    event->accept();
}

bool DesignerView::event(QEvent* event)
{
    if (event->type() == QEvent::Paint) {
        QRectF r(rect());
        QPainter p(this);
        p.setPen("#b6b6b6");
        p.drawLine(r.topLeft(), r.bottomLeft());
        p.drawLine(r.topRight(), r.bottomRight());
        return true;
    }
    return QGraphicsView::event(event);
}

void DesignerView::keyPressEvent(QKeyEvent* event)
{
    // check for autorepeat to avoid a stoped space panning by leave event to be restarted
    if (!event->isAutoRepeat() && m_panningState == Panning::NotStarted
            && event->key() == Qt::Key_Space) {
        startPanning(event);
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void DesignerView::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()
            && m_panningState == Panning::SpaceKeyStarted) {
        stopPanning(event);
    }
    QGraphicsView::keyReleaseEvent(event);
}

void DesignerView::mousePressEvent(QMouseEvent* event)
{
    if (m_panningState == Panning::NotStarted) {
        if (event->buttons().testFlag(Qt::MiddleButton))
            startPanning(event);
        else
            QGraphicsView::mousePressEvent(event);
    }
}

void DesignerView::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(QGraphicsView);
    // not sure why buttons() are empty here, but we have that information from the enum
    if (m_panningState == Panning::MouseWheelStarted)
        stopPanning(event);
    else
        QGraphicsView::mouseReleaseEvent(event);
    d->mousePressButton = Qt::NoButton;
}

void DesignerView::wheelEvent(QWheelEvent* event)
{
    // TODO: Implement a wheel zoom in/out for scene
    if (event->modifiers().testFlag(Qt::ControlModifier))
        event->ignore();
    else
        QGraphicsView::wheelEvent(event);
}

bool DesignerView::eventFilter(QObject* watched, QEvent* event)
{
    if (m_panningState != Panning::NotStarted) {
        if (event->type() == QEvent::Leave
                && m_panningState == Panning::SpaceKeyStarted) {
            // there is no way to keep the cursor so we stop panning here
            stopPanning(event);
        }
        if (event->type() == QEvent::MouseMove) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (!m_panningStartPosition.isNull()) {
                horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
                                                (mouseEvent->x() - m_panningStartPosition.x()));
                verticalScrollBar()->setValue(verticalScrollBar()->value() -
                                              (mouseEvent->y() - m_panningStartPosition.y()));
            }
            m_panningStartPosition = mouseEvent->pos();
            event->accept();
            return true;
        }
    }
    return QGraphicsView::eventFilter(watched, event);
}
