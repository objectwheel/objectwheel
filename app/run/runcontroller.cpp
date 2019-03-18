#include <runcontroller.h>
#include <runpane.h>
#include <pushbutton.h>
#include <rundevicesbutton.h>
#include <runprogressbar.h>
#include <runmanager.h>
#include <projectmanager.h>
#include <windowmanager.h>
#include <welcomewindow.h>
#include <preferenceswindow.h>
#include <generalsettings.h>
#include <interfacesettings.h>
#include <utilityfunctions.h>

namespace {
const char* g_welcomeMessage = "<b>Ready</b>  |  Welcome to Objectwheel (Beta)";
const char* g_userStoppedRunningMessage = "<b>Stopped</b>  |  Execution stopped at ";
const char* g_appCrashedMessage = "<b>Crashed</b>  |  Application crashed at ";
const char* g_finishedRunningMessage = "<b>Finished</b>  |  Application closed at ";
const char* g_startRunningMessage = "<b>Starting</b> interpretation...";
const char* g_runningMessage = "<b>Running</b> on ";
}

RunController::RunController(RunPane* runPane, QObject* parent) : QObject(parent)
    , m_runPane(runPane)
{
    connect(m_runPane->runButton(), &PushButton::clicked, this, &RunController::onRunButtonClick);
    connect(m_runPane->stopButton(), &PushButton::clicked, &RunManager::terminate);
    connect(m_runPane->projectsButton(), &PushButton::clicked, this, [=] {
        WindowManager::welcomeWindow()->show();
        WindowManager::welcomeWindow()->raise();
    });
    connect(m_runPane->preferencesButton(), &PushButton::clicked, this, [=] {
        WindowManager::preferencesWindow()->show();
        WindowManager::preferencesWindow()->raise();
    });

    QObject::connect(RunManager::instance(), &RunManager::deviceConnected,
                     m_runPane->runDevicesButton(), [=] (const QString& uid) {
        m_runPane->runDevicesButton()->addDevice(RunManager::deviceInfo(uid));
        m_runPane->runDevicesButton()->setCurrentDevice(uid);
    });
    QObject::connect(RunManager::instance(), &RunManager::deviceDisconnected, [=] (const QString& uid) {
        if (UtilityFunctions::deviceUid(UtilityFunctions::localDeviceInfo()) == uid)
            return;
        if (m_runPane->runDevicesButton()->currentDevice() == uid)
            m_runPane->runDevicesButton()->setCurrentDevice(UtilityFunctions::deviceUid(UtilityFunctions::localDeviceInfo()));
        m_runPane->runDevicesButton()->removeDevice(uid);
    });

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] {
        m_runPane->runProgressBar()->setHtml(tr(g_welcomeMessage));
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->runProgressBar()->setColor(m_runPane->runProgressBar()->palette().buttonText().color());
    });
    connect(RunManager::instance(), &RunManager::projectStarted,
            this, [=] {
        m_runPane->runButton()->setEnabled(true);
        m_runPane->runProgressBar()->setHtml(tr(g_runningMessage) + tr("My Computer"));
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setColor("#30acff");
        m_runPane->runProgressBar()->setBusy(false);
    });
    connect(RunManager::instance(), &RunManager::projectFinished, this,
            [=] (int exitCode) {
        if (exitCode == 0) // User just closed the app
            m_runPane->runProgressBar()->setHtml(tr(g_finishedRunningMessage) + QTime::currentTime().toString());
        else { // The app has erros thus the interpreter shut itself down
            m_runPane->runProgressBar()->setHtml(tr(g_appCrashedMessage) + QTime::currentTime().toString());
            m_runPane->runProgressBar()->setProgress(100);
            m_runPane->runProgressBar()->setColor("#e05650");
        }
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->runButton()->setEnabled(true);
        m_runPane->stopButton()->setDisabled(true);
    });

    connect(RunManager::instance(), &RunManager::deviceDisconnected, this,
            [=] () {
        m_runPane->runProgressBar()->setHtml(tr(g_appCrashedMessage) + QTime::currentTime().toString());
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setColor("#e05650");
        m_runPane->runButton()->setEnabled(true);
        m_runPane->stopButton()->setDisabled(true);
    });

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    QLinearGradient gradient({0.5, 0.0}, {0.5, 1.0});
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, settings->topBarColor.lighter(110));
    gradient.setColorAt(1, settings->topBarColor.darker(108));
    QPalette palette(m_runPane->palette());
    palette.setBrush(QPalette::Window, gradient);
    m_runPane->setPalette(palette);

    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged, this, [this] {
        const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
        QLinearGradient gradient({0.0, 0.0}, {0.0, 1.0});
        gradient.setCoordinateMode(QGradient::ObjectMode);
        gradient.setColorAt(0, settings->topBarColor.lighter(110));
        gradient.setColorAt(1, settings->topBarColor.darker(108));
        QPalette palette(m_runPane->palette());
        palette.setBrush(QPalette::Window, gradient);
        m_runPane->setPalette(palette);
    });
}

void RunController::discharge()
{
    RunManager::terminate();
    m_runPane->runButton()->setEnabled(true);
    m_runPane->stopButton()->setDisabled(true);
}

void RunController::onRunButtonClick()
{
    RunManager::terminate();

    m_runPane->runProgressBar()->setBusy(true);
    m_runPane->runProgressBar()->setProgress(40);
    m_runPane->runProgressBar()->setHtml(tr(g_startRunningMessage));
    RunManager::execute(m_runPane->runDevicesButton()->currentDevice(), ProjectManager::dir());

    m_runPane->runButton()->setDisabled(true);
    m_runPane->stopButton()->setEnabled(true);

    emit ran();
}
