#ifndef FRONTEND_H
#define FRONTEND_H

#include <windowmanager.h>
#include <mainwindow.h>
#include <designerwidget.h>

#define wM (WindowManager::instance())
#define mW ((MainWindow*)wM->get(WindowManager::Main))
#define dW ((DesignerWidget*)mW->designerWidget())

#endif // FRONTEND_H
