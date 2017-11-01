#include <savetransaction.h>
#include <control.h>
#include <controlwatcher.h>
#include <savemanager.h>

#define sM (SaveManager::instance())
#define cW (ControlWatcher::instance())

SaveTransaction::SaveTransaction()
{
    connect(cW, SIGNAL(zChanged()),
      this, SLOT(processZ(Control*)));
    connect(cW, SIGNAL(parentChanged()),
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
    sM->setProperty(control, "x",
      control->x());
    sM->setProperty(control, "y",
      control->y());
    sM->setProperty(control, "width",
      control->size().width());
    sM->setProperty(control, "height",
      control->size().height());
}

void SaveTransaction::processParent(Control* control)
{
    if (control->parentControl()) {
        sM->moveControl(control,
          control->parentControl());
        processGeometry(control);
    }
}

void SaveTransaction::processZ(Control* control)
{
    sM->setProperty(control, "z",
      control->zValue());
}
