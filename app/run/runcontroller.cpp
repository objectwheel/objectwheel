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

#include <QApplication>
#include <QTime>

RunController::RunController(RunPane* runPane, QObject* parent) : QObject(parent)
  , m_runScheduled(false)
  , m_appManuallyTerminated(false)
  , m_runPane(runPane)
{    
    onInterfaceSettingsChange();

    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged,
            this, &RunController::onInterfaceSettingsChange);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &RunController::discharge);
    connect(m_runPane->projectsButton(), &PushButton::clicked,
            this, &RunController::onProjectsButtonClick);
    connect(m_runPane->preferencesButton(), &PushButton::clicked,
            this, &RunController::onPreferencesButtonClick);
    connect(m_runPane->runButton(), &PushButton::clicked,
            this, &RunController::onRunButtonClick);
    connect(m_runPane->stopButton(), &PushButton::clicked,
            this, &RunController::onStopButtonClick);

    connect(RunManager::instance(), &RunManager::processStarted,
            this, &RunController::onProcessStart);
    connect(RunManager::instance(), &RunManager::processErrorOccurred,
            this, &RunController::onProcessErrorOccur);
    connect(RunManager::instance(), &RunManager::processFinished,
            this, &RunController::onProcessFinish);

    connect(RunManager::instance(), &RunManager::deviceConnected,
            this, &RunController::onDeviceConnect);
    connect(RunManager::instance(), &RunManager::deviceDisconnected,
            this, &RunController::onDeviceDisconnect);
    connect(RunManager::instance(), &RunManager::deviceUploadProgress,
            this, &RunController::onDeviceUploadProgress);
    connect(RunManager::instance(), &RunManager::deviceStarted,
            this, &RunController::onProcessStart);
    connect(RunManager::instance(), &RunManager::deviceErrorOccurred,
            this, &RunController::onProcessErrorOccur);
    connect(RunManager::instance(), &RunManager::deviceFinished,
            this, &RunController::onProcessFinish);
}

void RunController::discharge()
{
    m_runScheduled = false;
    m_appManuallyTerminated = false;
    RunManager::sendTerminate();
    m_runPane->stopButton()->setDisabled(true);
    m_runPane->runProgressBar()->setBusy(false);
    m_runPane->runProgressBar()->setProgress(0);
    m_runPane->runProgressBar()->setProgressColor(QColor());
    m_runPane->runProgressBar()->setText(progressBarMessageFor(Welcome));
}

void RunController::onInterfaceSettingsChange()
{
    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    QPalette palette(m_runPane->palette());
    QLinearGradient gradient({0.0, 0.0}, {0.0, 1.0});
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, settings->topBarColor.lighter(106));
    gradient.setColorAt(1, settings->topBarColor.darker(107));
    palette.setBrush(QPalette::Window, gradient);
    m_runPane->setPalette(palette);
}

void RunController::onProjectsButtonClick()
{
    WindowManager::welcomeWindow()->show();
    WindowManager::welcomeWindow()->raise();
}

void RunController::onPreferencesButtonClick()
{
    WindowManager::preferencesWindow()->show();
    WindowManager::preferencesWindow()->raise();
}

void RunController::onRunButtonClick()
{
    if (m_runPane->stopButton()->isEnabled())
        m_runScheduled = true;
    else
        m_runScheduled = false;
    m_appManuallyTerminated = false;
    m_runPane->runProgressBar()->setBusy(true);
    m_runPane->runProgressBar()->setProgress(1);
    m_runPane->runProgressBar()->setProgressColor(QColor());
    m_runPane->runProgressBar()->setText(progressBarMessageFor(Starting));
    m_runPane->stopButton()->setEnabled(true);
    RunManager::sendExecute(m_runPane->runDevicesButton()->currentDevice(), ProjectManager::dir());
    emit ran();
}

void RunController::onStopButtonClick()
{
    m_appManuallyTerminated = true;
    RunManager::sendTerminate();
}

void RunController::onProcessStart()
{
    m_runPane->runProgressBar()->setBusy(false);
    m_runPane->runProgressBar()->setProgress(100);
    m_runPane->runProgressBar()->setProgressColor("#247dd6");
    m_runPane->runProgressBar()->setText(progressBarMessageFor(Running));
}

void RunController::onProcessErrorOccur(QProcess::ProcessError error, const QString& errorString)
{
    if (error == QProcess::Crashed && m_runScheduled)
        return;
    m_runScheduled = false;
    m_runPane->runProgressBar()->setProgressColor("#e05650");
    if (error == QProcess::FailedToStart) {
        m_runPane->stopButton()->setEnabled(false);
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setText(progressBarMessageFor(Failure, errorString));
    } else if (m_appManuallyTerminated) {
        m_runPane->runProgressBar()->setText(progressBarMessageFor(Stopped));
    } else {
        m_runPane->runProgressBar()->setText(progressBarMessageFor(Crashed));
    }
}

void RunController::onProcessFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit && m_runScheduled) {
        m_runScheduled = false;
        return;
    }
    m_runScheduled = false;
    if (exitStatus != QProcess::CrashExit) {
        if (exitCode == EXIT_FAILURE) {
            m_runPane->runProgressBar()->setProgressColor("#e05650");
            m_runPane->runProgressBar()->setText(progressBarMessageFor(Crashed));
        } else {
            m_runPane->runProgressBar()->setText(progressBarMessageFor(Finished));
        }
    }
    m_runPane->stopButton()->setEnabled(false);
}

void RunController::onDeviceConnect(const QString& uid)
{
    m_runPane->runDevicesButton()->addDevice(RunManager::deviceInfo(uid));
    if (!m_runPane->stopButton()->isEnabled())
        m_runPane->runDevicesButton()->setCurrentDevice(uid);
}

void RunController::onDeviceDisconnect(const QVariantMap& deviceInfo)
{
    const QString& deviceUid = UtilityFunctions::deviceUid(deviceInfo);
    if (m_runPane->runDevicesButton()->currentDevice() == deviceUid)
        m_runPane->runDevicesButton()->setCurrentDevice(UtilityFunctions::deviceUid(UtilityFunctions::localDeviceInfo()));
    m_runPane->runDevicesButton()->removeDevice(deviceUid);

    if (RunManager::recentDevice() == deviceUid) {
        m_runScheduled = false;
        RunManager::scheduleUploadCancelation();
        if (m_runPane->runProgressBar()->progress() > 0
                && m_runPane->runProgressBar()->progress() < 100) {
            m_runPane->runProgressBar()->setProgressColor("#e05650");
            m_runPane->runProgressBar()->setBusy(false);
            m_runPane->runProgressBar()->setProgress(100);
        }
        if (m_runPane->stopButton()->isEnabled()) {
            m_runPane->stopButton()->setEnabled(false);
            m_runPane->runProgressBar()->setText(
                        progressBarMessageFor(Disconnected, UtilityFunctions::deviceName(deviceInfo)));
        }
    }
}

void RunController::onDeviceUploadProgress(int progress)
{
    m_runPane->runProgressBar()->setProgress(progress * 0.99);
}

QString RunController::progressBarMessageFor(MessageKind kind, const QString& arg)
{
    using namespace UtilityFunctions;
    static const char* msgWelcome  = QT_TR_NOOP("<b>Ready</b>  |  Welcome to Objectwheel (Beta)");
    static const char* msgStarting = QT_TR_NOOP("<b>Starting</b> the application....");
    static const char* msgFailure  = QT_TR_NOOP("<b>System Failure</b>  |  %1 at %2");
    static const char* msgDisconnected = QT_TR_NOOP("<b>Disconnected</b>  |  Connection lost to <i>%1</i> at %2");
    static const char* msgRunning  = QT_TR_NOOP("<b>Running</b> on %1");
    static const char* msgCrashed  = QT_TR_NOOP("<b>Crashed</b>  |  The application crashed at %1");
    static const char* msgStopped  = QT_TR_NOOP("<b>Stopped</b>  |  The application terminated at %1");
    static const char* msgFinished = QT_TR_NOOP("<b>Finished</b>  |  The application exited at %1");

    QString message = "<p style='white-space:pre'>" + ProjectManager::name() + "  :  ";

    switch (kind) {
    case Welcome:
        message += tr(msgWelcome);
        break;
    case Starting:
        message += tr(msgStarting);
        break;
    case Failure:
        message += tr(msgFailure).arg(arg).arg(QTime::currentTime().toString());
        break;
    case Disconnected:
        message += tr(msgDisconnected).arg(arg).arg(QTime::currentTime().toString());
        break;
    case Running:
        message += tr(msgRunning).arg(deviceName(RunManager::deviceInfo(RunManager::recentDevice())));
        break;
    case Crashed:
        message += tr(msgCrashed).arg(QTime::currentTime().toString());
        break;
    case Stopped:
        message += tr(msgStopped).arg(QTime::currentTime().toString());
        break;
    case Finished:
        message += tr(msgFinished).arg(QTime::currentTime().toString());
        break;
    default:
        break;
    }

    return message += "</p>";
}