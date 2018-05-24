#include <backendmanager.h>
#include <toolsbackend.h>
#include <userbackend.h>
#include <projectbackend.h>
#include <exposerbackend.h>
#include <windowmanager.h>
#include <authenticator.h>
#include <previewerbackend.h>
#include <savetransaction.h>
#include <editorbackend.h>
#include <mainwindow.h>
#include <QMessageBox>

#include <coreplugin/coreconstants.h>
#include <theme/theme_p.h>
#include <coreplugin/themechooser.h>
#include <coreplugin/helpmanager.h>

using namespace Core;

Authenticator* BackendManager::s_authenticator = nullptr;
HelpManager* BackendManager::s_helpManager = nullptr;
EditorBackend* BackendManager::s_editorBackend = nullptr;

BackendManager::BackendManager()
{
    s_authenticator = new Authenticator(this);
    s_helpManager = new HelpManager(this);
    HelpManager::setupHelpManager();
    Utils::setCreatorTheme(Internal::ThemeEntry::createTheme(Constants::DEFAULT_THEME));
    connect(qApp, &QCoreApplication::aboutToQuit, s_helpManager, &HelpManager::aboutToShutdown);

    s_editorBackend = new EditorBackend(this);

    connect(ProjectBackend::instance(), &ProjectBackend::started,
            static_cast<MainWindow*>(WindowManager::instance()->get(WindowManager::Main)), &MainWindow::reset);
    connect(ProjectBackend::instance(), &ProjectBackend::started, this, &BackendManager::onProjectStart);
    connect(UserBackend::instance(), &UserBackend::aboutToStop, this, &BackendManager::onSessionStop);

    SaveTransaction::instance();
    Authenticator::setHost(QUrl(APP_WSSSERVER));

    if (!PreviewerBackend::instance()->init()) {
        QMessageBox::critical(
            nullptr,
            tr("Error"),
            tr("Unable to start Objectwheel Previewing Service")
        );
    }
}

void BackendManager::init()
{
    static BackendManager instance;
    Q_UNUSED(instance)
}

QSettings* BackendManager::settings(QSettings::Scope scope)
{
    static QSettings settings, globalSettings;
    return scope == QSettings::UserScope ? &settings : &globalSettings;
}

QString BackendManager::resourcePath()
{
    return tr(":");
}

QString BackendManager::userResourcePath()
{
    return tr(":");
}

void BackendManager::onSessionStop()
{
    ProjectBackend::instance()->stop();
}

void BackendManager::onProjectStart()
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
