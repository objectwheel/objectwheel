#include <savetransaction.h>
#include <control.h>
#include <controlwatcher.h>
#include <savebackend.h>
#include <saveutils.h>
#include <previewerbackend.h>

SaveTransaction::SaveTransaction()
{
    connect(cW, SIGNAL(zValueChanged(Control*)), this, SLOT(processZ(Control*)));
    connect(cW, SIGNAL(parentChanged(Control*)), this, SLOT(processParent(Control*)));
    connect(cW, SIGNAL(geometryChanged(Control*)), this, SLOT(processGeometry(Control*)));
}

SaveTransaction* SaveTransaction::instance()
{
    static SaveTransaction instance;
    return &instance;
}

void SaveTransaction::processGeometry(Control* control)
{
    if ((!control->gui() || control->hasErrors()) && !control->form()) {
        SaveUtils::setX(control->dir(), control->x());
        SaveUtils::setY(control->dir(), control->y());
        return;
    }

    if (!control->form()) {
        SaveBackend::instance()->setProperty(control, "x", QString::number(control->x()));
        SaveBackend::instance()->setProperty(control, "y", QString::number(control->y()));
    }

    SaveBackend::instance()->setProperty(control, "width", QString::number(control->size().width()));
    SaveBackend::instance()->setProperty(control, "height", QString::number(control->size().height()));

    if (!control->form()) {
        PreviewerBackend::instance()->updateCache(control->uid(), "x", control->x());
        PreviewerBackend::instance()->updateCache(control->uid(), "y", control->y());
    }
}

void SaveTransaction::processParent(Control* control)
{
    if (!control->parentControl() || (!control->parentControl()->gui() && control->gui()))
        return;

    SaveBackend::instance()->moveControl(control, control->parentControl());
    processGeometry(control);
}

void SaveTransaction::processZ(Control* control)
{
    if (control->hasErrors() || !control->gui())
        return;

    SaveBackend::instance()->setProperty(control, "z", QString::number(control->zValue()));
}
