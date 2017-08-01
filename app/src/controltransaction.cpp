#include <controltransaction.h>
#include <savemanager.h>
#include <control.h>
#include <windowscene.h>

ControlTransaction::ControlTransaction(Control* watched, QObject *parent)
    : QObject(parent)
    , _watched(watched)
{
    connect(_watched, SIGNAL(geometryChanged()), this, SLOT(flushGeometryChange()));
    connect(_watched, SIGNAL(parentChanged()), this, SLOT(flushParentChange()));
    connect(_watched, SIGNAL(zChanged()), this, SLOT(flushZChange()));
    connect(_watched, SIGNAL(idChanged(const QString&)), this, SLOT(flushIdChange(const QString&)));
}

void ControlTransaction::flushGeometryChange()
{
    if (_watched->id().isEmpty())
        return;

    SaveManager::setVariantProperty(_watched->id(), "x", _watched->x());
    SaveManager::setVariantProperty(_watched->id(), "y", _watched->y());
    SaveManager::setVariantProperty(_watched->id(), "width", _watched->size().width());
    SaveManager::setVariantProperty(_watched->id(), "height", _watched->size().height());
}

void ControlTransaction::flushParentChange()
{
    if (_watched->parentControl() == nullptr || _watched->id().isEmpty())
        return;

    SaveManager::addParentalRelationship(_watched->id(), _watched->parentControl()->id());
}

void ControlTransaction::flushZChange()
{
    if (_watched->id().isEmpty())
        return;

    SaveManager::setVariantProperty(_watched->id(), "z", _watched->zValue());
}

void ControlTransaction::flushIdChange(const QString& prevId)
{
    if (_watched->id().isEmpty() || prevId.isEmpty())
        return;

    SaveManager::changeSave(prevId, _watched->id());
}
