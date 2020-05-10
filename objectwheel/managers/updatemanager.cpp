#include <updatemanager.h>

UpdateManager* UpdateManager::s_instance = nullptr;

UpdateManager::UpdateManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

UpdateManager::~UpdateManager()
{
    s_instance = nullptr;
}

UpdateManager* UpdateManager::instance()
{
    return s_instance;
}

