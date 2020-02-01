#include <buildscontroller.h>
#include <buildspane.h>

BuildsController::BuildsController(BuildsPane* buildsPane, QObject* parent) : QObject(parent)
  , m_buildsPane(buildsPane)
{
}

void BuildsController::charge()
{
    // TODO
}

void BuildsController::discharge()
{
    // TODO
}
