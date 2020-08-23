#ifndef MACOPERATIONS_H
#define MACOPERATIONS_H

class QString;
class QMainWindow;

namespace MacOperations {

void enableIdleSystemSleep();
void disableIdleSystemSleep();
void removeTitleBar(QMainWindow* mainWindow);

}

#endif // MACOPERATIONS_H