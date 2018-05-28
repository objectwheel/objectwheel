#include <controlwatcher.h>

ControlWatcher* ControlWatcher::s_instance = nullptr;

ControlWatcher::ControlWatcher(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ControlWatcher::~ControlWatcher()
{
    s_instance = nullptr;
}

ControlWatcher* ControlWatcher::instance()
{
    return s_instance;
}
