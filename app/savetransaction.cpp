#include <savetransaction.h>
#include <control.h>
#include <controlwatcher.h>
#include <savebackend.h>
#include <saveutils.h>
#include <previewerbackend.h>

SaveTransaction::SaveTransaction(QObject* parent) : QObject(parent)
{
    connect(cW, &ControlWatcher::zValueChanged, this, &SaveTransaction::processZ);
    connect(cW, &ControlWatcher::parentChanged, this, &SaveTransaction::processParent);
    connect(cW, &ControlWatcher::geometryChanged, this, &SaveTransaction::processGeometry);
}

void SaveTransaction::processGeometry(Control* control)
{
    if ((!control->gui() || control->hasErrors()) && !control->form()) {
        SaveUtils::setX(control->dir(), control->x());
        SaveUtils::setY(control->dir(), control->y());
        return;
    }

    if (!control->form()) {
        SaveBackend::setProperty(control, "x", QString::number(control->x()));
        SaveBackend::setProperty(control, "y", QString::number(control->y()));
    }

    SaveBackend::setProperty(control, "width", QString::number(control->size().width()));
    SaveBackend::setProperty(control, "height", QString::number(control->size().height()));

    if (!control->form()) {
        PreviewerBackend::updateCache(control->uid(), "x", control->x());
        PreviewerBackend::updateCache(control->uid(), "y", control->y());
    }
}

void SaveTransaction::processParent(Control* control)
{
    if (!control->parentControl() || (!control->parentControl()->gui() && control->gui()))
        return;

    SaveBackend::moveControl(control, control->parentControl());
    processGeometry(control);
}

void SaveTransaction::processZ(Control* control)
{
    if (control->hasErrors() || !control->gui())
        return;

    SaveBackend::setProperty(control, "z", QString::number(control->zValue()));
}
