#include <inactivitywatcher.h>

InactivityWatcher* InactivityWatcher::s_instance = nullptr;
bool InactivityWatcher::s_deactivated = true;
int InactivityWatcher::s_secsLimit = 0;
QTimer InactivityWatcher::s_inactivityTimer;

InactivityWatcher::InactivityWatcher(int secsLimit, QObject* parent) : QObject(parent)
{
    Q_ASSERT(secsLimit > 0);
    s_instance = this;
    s_secsLimit = secsLimit;
    s_inactivityTimer.setSingleShot(true);
    s_inactivityTimer.callOnTimeout(this, &InactivityWatcher::onTimeout);
}

InactivityWatcher::~InactivityWatcher()
{
    s_instance = nullptr;
}

InactivityWatcher* InactivityWatcher::instance()
{
    return s_instance;
}

int InactivityWatcher::secsLimit()
{
    return s_secsLimit;
}

void InactivityWatcher::activate(bool active)
{
    if (active) {
        if (s_inactivityTimer.isActive())
            s_inactivityTimer.stop();
        if (s_deactivated) {
            s_deactivated = false;
            emit s_instance->activated();
        }
    } else if (!s_inactivityTimer.isActive() && !s_deactivated) {
        s_inactivityTimer.start(1000 * s_secsLimit);
    }
}

void InactivityWatcher::onTimeout()
{
    s_deactivated = true;
    emit deactivated();
}
