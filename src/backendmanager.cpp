#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <savebackend.h>
#include <previewbackend.h>

#include <designerwidget.h>

BackendManager::BackendManager()
{
    connect(ProjectBackend::instance(), SIGNAL(started()),
      SLOT(handleProjectStart()));
}

BackendManager* BackendManager::instance()
{
    static BackendManager instance;
    return &instance;
}

void BackendManager::handleProjectStart() const
{
    // mainWindow->clearStudio(); //FIXME
    SaveBackend::exposeProject();
    DesignerWidget::controlScene()->clearSelection();
    DesignerWidget::formScene()->clearSelection();
    ToolsBackend::instance()->downloadTools();
    // MainWindow::instance()->setWindowTitle //FIXME
    // (QString(APP_NAME) + " - [" + projectname + "]");
}
