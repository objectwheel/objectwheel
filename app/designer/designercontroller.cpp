#include <designercontroller.h>
#include <designerpane.h>
#include <designerview.h>
#include <designerscene.h>

DesignerController::DesignerController(DesignerPane* designerPane, QObject* parent) : QObject(parent)
  , m_designerPane(designerPane)
{
}

void DesignerController::discharge()
{
    m_designerPane->designerView()->scene()->clear();
}
