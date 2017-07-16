#include <controltransaction.h>
#include <savemanager.h>
#include <control.h>

ControlTransaction::ControlTransaction(Control* watched, QObject *parent)
    : QObject(parent)
    , _watched(watched)
{
    connect(_watched, SIGNAL(parentChanged()), this, SLOT(handleParentChange()));
    connect(_watched, SIGNAL(geometryChanged()), this, SLOT(handleGeometryChange()));
}

#include <QtConcurrent>
void ControlTransaction::handleGeometryChange()
{

//    auto ret = QtConcurrent::run((void (*)(const QString&, const QString&, const QVariant&))(&SaveManager::setVariantProperty),
//                                 _watched->id(),
//                                 QString("x"), QVariant(_watched->x()));


    SaveManager::setVariantProperty(_watched->id(), "x", _watched->x());
    SaveManager::setVariantProperty(_watched->id(), "y", _watched->x());
    SaveManager::setVariantProperty(_watched->id(), "width", _watched->size().width());
    SaveManager::setVariantProperty(_watched->id(), "height", _watched->size().height());
}

void ControlTransaction::handleParentChange()
{
    if (_watched->parentControl())
        SaveManager::addParentalRelationship(_watched->id(), _watched->parentControl()->id());
}
