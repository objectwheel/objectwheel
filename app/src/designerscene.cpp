#include <designerscene.h>
#include <control.h>

#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>

DesignerScene::DesignerScene(QObject *parent) : QGraphicsScene(parent)
{

}

void DesignerScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void DesignerScene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void DesignerScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
}

void DesignerScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();
    auto url = event->mimeData()->urls().at(0);
    auto control = new Control;
    control->setId("eben");
    control->setUrl(url);
    addItem(control);
}
