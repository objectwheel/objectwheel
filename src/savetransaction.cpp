#include <savetransaction.h>
#include <control.h>
#include <controlwatcher.h>
#include <savebackend.h>
#include <fit.h>

#define cW (ControlWatcher::instance())

SaveTransaction::SaveTransaction()
{
    connect(cW, SIGNAL(zValueChanged(Control*)),
      this, SLOT(processZ(Control*)));
    connect(cW, SIGNAL(parentChanged(Control*)),
      this, SLOT(processParent(Control*)));
    connect(cW, SIGNAL(geometryChanged(Control*)),
      this, SLOT(processGeometry(Control*)));
}

SaveTransaction* SaveTransaction::instance()
{
    static SaveTransaction instance;
    return &instance;
}

void SaveTransaction::processGeometry(Control* control)
{
    SaveBackend::instance()->setProperty(control, "x", control->x());
    SaveBackend::instance()->setProperty(control, "y", control->y());
    SaveBackend::instance()->setProperty(control, "width", control->size().width() / fit::ratio());
    SaveBackend::instance()->setProperty(control, "height", control->size().height() / fit::ratio());
}

void SaveTransaction::processParent(Control* control)
{
    if (control->parentControl()) {
        SaveBackend::instance()->moveControl(control, control->parentControl());
        processGeometry(control);
    }
}

void SaveTransaction::processZ(Control* control)
{
    SaveBackend::instance()->setProperty(control, "z", control->zValue());
}
