#include <controltransaction.h>
#include <savemanager.h>
#include <control.h>

ControlTransaction::ControlTransaction(Control* watched, QObject *parent)
    : QObject(parent)
    , _watched(watched)
{
    connect(_watched, SIGNAL(geometryChanged()), this, SLOT(flushGeometryChange()));
    connect(_watched, SIGNAL(parentChanged()), this, SLOT(flushParentChange()));
}

void ControlTransaction::flushGeometryChange()
{
    SaveManager::setVariantProperty(_watched->id(), "x", _watched->x());
    SaveManager::setVariantProperty(_watched->id(), "y", _watched->x());
    SaveManager::setVariantProperty(_watched->id(), "width", _watched->size().width());
    SaveManager::setVariantProperty(_watched->id(), "height", _watched->size().height());
}

void ControlTransaction::flushParentChange()
{
    if (_watched->parentControl() && !_watched->id().isEmpty())
        SaveManager::addParentalRelationship(_watched->id(), _watched->parentControl()->id());
}
