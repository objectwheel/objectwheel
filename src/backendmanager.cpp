#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <savebackend.h>
#include <previewbackend.h>
#include <frontend.h>
#include <windowmanager.h>
#include <authenticator.h>

BackendManager::BackendManager()
{
    Authenticator::instance()->init(QUrl(APP_SERVER));

    new SaveBackend;
    new PreviewBackend;

    connect(ProjectBackend::instance(), SIGNAL(started()),
      WindowManager::instance()->get(WindowManager::Main), SLOT(clear()));
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
    SaveBackend::exposeProject();
//    dW->controlScene()->clearSelection();
//    dW->formScene()->clearSelection();
    ToolsBackend::instance()->downloadTools();
    // MainWindow::instance()->setWindowTitle //FIXME
    // (QString(APP_NAME) + " - [" + projectname + "]");
}
