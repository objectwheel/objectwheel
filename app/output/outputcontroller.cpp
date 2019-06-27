#include <outputcontroller.h>

OutputController::OutputController(OutputPane* outputPane, QObject* parent) : QObject(parent)
  , m_outputPane(outputPane)
{

}
