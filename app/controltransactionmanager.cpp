#include <controltransactionmanager.h>
#include <control.h>
#include <controlmonitoringmanager.h>
#include <savemanager.h>
#include <saveutils.h>
#include <controlpreviewingmanager.h>

ControlTransactionManager::ControlTransactionManager(QObject* parent) : QObject(parent)
{
    connect(ControlMonitoringManager::instance(), &ControlMonitoringManager::zValueChanged,
            this, &ControlTransactionManager::processZ);
    connect(ControlMonitoringManager::instance(), &ControlMonitoringManager::parentChanged,
            this, &ControlTransactionManager::processParent);
    connect(ControlMonitoringManager::instance(), &ControlMonitoringManager::geometryChanged,
            this, &ControlTransactionManager::processGeometry);
}

void ControlTransactionManager::processGeometry(Control* control)
{
    if ((!control->gui() || control->hasErrors()) && !control->form()) {
        SaveUtils::setX(control->dir(), control->x());
        SaveUtils::setY(control->dir(), control->y());
        return;
    }

    if (!control->form()) {
        SaveManager::setProperty(control, "x", QString::number(control->x()));
        SaveManager::setProperty(control, "y", QString::number(control->y()));
    }

    SaveManager::setProperty(control, "width", QString::number(control->size().width()));
    SaveManager::setProperty(control, "height", QString::number(control->size().height()));

    if (!control->form()) {
        ControlPreviewingManager::updateCache(control->uid(), "x", control->x());
        ControlPreviewingManager::updateCache(control->uid(), "y", control->y());
    }
}

void ControlTransactionManager::processParent(Control* control)
{
    if (!control->parentControl() || (!control->parentControl()->gui() && control->gui()))
        return;

    SaveManager::moveControl(control, control->parentControl());
    processGeometry(control);
}

void ControlTransactionManager::processZ(Control* control)
{
    if (control->hasErrors() || !control->gui())
        return;

    SaveManager::setProperty(control, "z", QString::number(control->zValue()));
}
