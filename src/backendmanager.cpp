#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <savebackend.h>
#include <previewbackend.h>
#include <designerwidget.h>

BackendManager::BackendManager()
{
    new SaveBackend;
    new PreviewBackend;

    connect(ProjectBackend::instance(), SIGNAL(started()),
      SLOT(handleProjectStart()));
    connect(UserBackend::instance(), SIGNAL(aboutToStop()),
      SLOT(handleSessionStop()));
}

BackendManager* BackendManager::instance()
{
    static BackendManager instance;
    return &instance;
}

void BackendManager::handleSessionStop() const
{
    ProjectBackend::instance()->stop();
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
