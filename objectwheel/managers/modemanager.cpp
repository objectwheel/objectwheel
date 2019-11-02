#include <modemanager.h>

ModeManager* ModeManager::s_instance = nullptr;
ModeManager::Mode ModeManager::s_mode = ModeManager::Designer;

ModeManager::ModeManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

ModeManager::~ModeManager()
{
    s_instance = nullptr;
}

ModeManager* ModeManager::instance()
{
    return s_instance;
}

ModeManager::Mode ModeManager::mode()
{
    return s_mode;
}

void ModeManager::setMode(ModeManager::Mode mode)
{
    if (mode != s_mode) {
        s_mode = mode;
        emit instance()->modeChanged(mode);
    }
}

