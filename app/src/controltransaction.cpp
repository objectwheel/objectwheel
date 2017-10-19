#include <controltransaction.h>
#include <savemanager.h>
#include <control.h>
#include <formscene.h>

ControlTransaction::ControlTransaction(Control* watched, QObject *parent)
    : QObject(parent)
    , _watched(watched)
    , _transactionsEnabled(false)
    , _geometryTransactionsEnabled(true)
    , _parentTransactionsEnabled(true)
    , _zTransactionsEnabled(true)
    , _activated(false)
{
    connect(_watched, SIGNAL(geometryChanged()), SLOT(flushGeometryChange()));
    connect(_watched, SIGNAL(parentChanged()), SLOT(flushParentChange()));
    connect(_watched, SIGNAL(zChanged()), SLOT(flushZChange()));

    connect(SaveManager::instance(), &SaveManager::parserRunningChanged,
      this, [this](bool running) {
        if (_watched && _activated && running == false) {
            _activated = false;
            _watched->refresh();
        }
    });
}

void ControlTransaction::setGeometryTransactionsEnabled(bool value)
{
    _geometryTransactionsEnabled = value;
}

void ControlTransaction::setParentTransactionsEnabled(bool value)
{
    _parentTransactionsEnabled = value;
}

void ControlTransaction::setZTransactionsEnabled(bool value)
{
    _zTransactionsEnabled = value;
}

void ControlTransaction::setTransactionsEnabled(bool value)
{
    _transactionsEnabled = value;
}

void ControlTransaction::flushGeometryChange()
{
    if (_watched || _watched->id().isEmpty() ||
        !_geometryTransactionsEnabled ||
        !_transactionsEnabled)
        return;

    if (!_watched->form()) {
        SaveManager::setProperty(_watched, "x", _watched->x());
        SaveManager::setProperty(_watched, "y", _watched->y());
    }
    SaveManager::setProperty(_watched, "width", _watched->form() ?
        int(_watched->size().width()) : _watched->size().width());
    SaveManager::setProperty(_watched, "height", _watched->form() ?
        int(_watched->size().height()) : _watched->size().height());
    _activated = true;
}

void ControlTransaction::flushParentChange()
{
    if (_watched || _watched->parentControl() == nullptr ||
        _watched->id().isEmpty() ||
        !_parentTransactionsEnabled ||
        !_transactionsEnabled)
        return;

    SaveManager::moveControl(_watched, _watched->parentControl());
    flushGeometryChange();
    _activated = true;
}

void ControlTransaction::flushZChange()
{
    if (_watched || _watched->id().isEmpty() ||
        !_zTransactionsEnabled ||
        !_transactionsEnabled)
        return;

    SaveManager::setProperty(_watched, "z", _watched->zValue());
    _activated = true;
}

bool ControlTransaction::transactionsEnabled() const
{
    return _transactionsEnabled;
}
