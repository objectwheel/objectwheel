#include <designerpane.h>

#include <designerview.h>
#include <designerscene.h>
#include <utilsicons.h>
#include <controlcreationmanager.h>
#include <controlpropertymanager.h>
#include <controlremovingmanager.h>
#include <saveutils.h>
#include <transparentstyle.h>
#include <signalchooserdialog.h>
#include <utilityfunctions.h>
#include <qmlcodeeditorwidget.h>
#include <parserutils.h>
#include <projectmanager.h>
#include <qmlcodeeditor.h>
#include <qmlcodedocument.h>
#include <designersettings.h>
#include <scenesettings.h>
#include <anchoreditor.h>
#include <form.h>

#include <QScrollBar>
#include <QMenu>
#include <QDir>
#include <QToolBar>
#include <QToolButton>
#include <QPainter>
#include <QMessageBox>
#include <QComboBox>
#include <QInputDialog>

DesignerPane::DesignerPane(QWidget *parent) : QWidget(parent)
{

}

void DesignerPane::setZoomLevel(qreal zoomLevel)
{
    SceneSettings* settings = DesignerSettings::sceneSettings();
    qreal recentZoomLevel = matrix().m11();
    if (recentZoomLevel != zoomLevel) {
        resetTransform();
        scale((1.0 / recentZoomLevel) * zoomLevel, (1.0 / recentZoomLevel) * zoomLevel);
        settings->sceneZoomLevel = zoomLevel;
        settings->write();
    }
}

QSize DesignerPane::sizeHint() const
{
    return QSize(680, 680);
}

void DesignerPane::contextMenuEvent(QContextMenuEvent* event)
{
//    QGraphicsView::contextMenuEvent(event);

//    auto selectedControls = scene()->selectedControls();
//    selectedControls.removeOne(scene()->currentForm());

//    if (selectedControls.isEmpty()) {
//        m_sendBackAct->setDisabled(true);
//        m_bringFrontAct->setDisabled(true);
//        m_cutAct->setDisabled(true);
//        m_copyAct->setDisabled(true);
//        m_deleteAct->setDisabled(true);
//    } else {
//        m_sendBackAct->setDisabled(false);
//        m_bringFrontAct->setDisabled(false);
//        m_cutAct->setDisabled(false);
//        m_copyAct->setDisabled(false);
//        m_deleteAct->setDisabled(false);
//    }
    // FIXME : m_menu->exec(event->globalPos());
}

bool DesignerPane::eventFilter(QObject *watched, QEvent *event)
{
    if (m_isPanning != Panning::NotStarted) {
        if (event->type() == QEvent::Leave && m_isPanning == Panning::SpaceKeyStarted) {
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

void DesignerPane::wheelEvent(QWheelEvent *event)
{
    // TODO: Implement a wheel zoom in/out for scene
    if (event->modifiers().testFlag(Qt::ControlModifier))
        event->ignore();
    else
        QGraphicsView::wheelEvent(event);
}

void DesignerPane::mousePressEvent(QMouseEvent *event)
{
    if (m_isPanning == Panning::NotStarted) {
        if (event->buttons().testFlag(Qt::MiddleButton))
            startPanning(event);
        else
            QGraphicsView::mousePressEvent(event);
    }
}

void DesignerPane::mouseReleaseEvent(QMouseEvent *event)
{
    // not sure why buttons() are empty here, but we have that information from the enum
    if (m_isPanning == Panning::MouseWheelStarted)
        stopPanning(event);
    else
        QGraphicsView::mouseReleaseEvent(event);
}

void DesignerPane::keyPressEvent(QKeyEvent *event)
{
    // check for autorepeat to avoid a stoped space panning by leave event to be restarted
    if (!event->isAutoRepeat() && m_isPanning == Panning::NotStarted && event->key() == Qt::Key_Space) {
        startPanning(event);
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void DesignerPane::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat() && m_isPanning == Panning::SpaceKeyStarted)
        stopPanning(event);

    QGraphicsView::keyReleaseEvent(event);
}

void DesignerPane::startPanning(QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
        m_isPanning = Panning::SpaceKeyStarted;
    else
        m_isPanning = Panning::MouseWheelStarted;
    viewport()->setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void DesignerPane::stopPanning(QEvent *event)
{
    m_isPanning = Panning::NotStarted;
    m_panningStartPosition = QPoint();
    viewport()->unsetCursor();
    event->accept();
}
