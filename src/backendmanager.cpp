#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <savebackend.h>
#include <frontend.h>
#include <windowmanager.h>
#include <authenticator.h>
#include <previewerbackend.h>

BackendManager::BackendManager()
{
    Authenticator::instance()->init(QUrl(APP_WSSSERVER));

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
    PreviewerBackend::instance()->start();

    SaveBackend::instance()->exposeProject();
//    dW->controlScene()->clearSelection();
//    dW->formScene()->clearSelection();
    ToolsBackend::instance()->downloadTools();
    // MainWindow::instance()->setWindowTitle //FIXME
    // (QString(APP_NAME) + " - [" + projectname + "]");
}
