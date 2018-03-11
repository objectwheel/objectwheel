#ifndef FRONTEND_H
#define FRONTEND_H

#include <windowmanager.h>
#include <mainwindow.h>
#include <centralwidget.h>

#define wM (WindowManager::instance())
#define mW ((MainWindow*)wM->get(WindowManager::Main))
#define dW ((CentralWidget*)mW->centralWidget())

#endif // FRONTEND_H
