#include <controlwatcher.h>

ControlWatcher* ControlWatcher::instance()
{
    static ControlWatcher instance;
    return &instance;
}
