#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <exposerbackend.h>
#include <windowmanager.h>
#include <authenticator.h>
#include <previewerbackend.h>
#include <savetransaction.h>
#include <QMessageBox>

BackendManager::BackendManager()
{
    SaveTransaction::instance();
    Authenticator::instance()->init(QUrl(APP_WSSSERVER));

    if (!PreviewerBackend::instance()->init()) {
        QMessageBox::critical(
            nullptr,
            tr("Error"),
            tr("Unable to start Objectwheel Previewing Service")
        );
    }

    connect(ProjectBackend::instance(), SIGNAL(started()), WindowManager::instance()->get(WindowManager::Main), SLOT(reset()));
    connect(ProjectBackend::instance(), SIGNAL(started()), SLOT(handleProjectStart()));
    connect(UserBackend::instance(), SIGNAL(aboutToStop()), SLOT(handleSessionStop()));
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
    PreviewerBackend::instance()->restart();
    PreviewerBackend::instance()->requestInit(ProjectBackend::instance()->dir());
    ExposerBackend::instance()->exposeProject();
//    dW->controlScene()->clearSelection();
//    dW->designerScene()->clearSelection();
    ToolsBackend::instance()->downloadTools();
    // MainWindow::instance()->setWindowTitle //FIXME
    // (QString(APP_NAME) + " - [" + projectname + "]");
}