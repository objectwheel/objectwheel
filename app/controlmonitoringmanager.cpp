#include <controlmonitoringmanager.h>

ControlMonitoringManager* ControlMonitoringManager::s_instance = nullptr;

ControlMonitoringManager::ControlMonitoringManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ControlMonitoringManager::~ControlMonitoringManager()
{
    s_instance = nullptr;
}

ControlMonitoringManager* ControlMonitoringManager::instance()
{
    return s_instance;
}
