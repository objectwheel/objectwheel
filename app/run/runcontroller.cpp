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

RunController::RunController(RunPane* runPane, QObject* parent) : QObject(parent)
  , m_appManuallyTerminated(false)
  , m_runPane(runPane)
{    
    onInterfaceSettingsChange();

    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged,
            this, &RunController::onInterfaceSettingsChange);
    connect(ProjectManager::instance(), &ProjectManager::started, this, [=] {
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->runProgressBar()->setProgress(0);
        m_runPane->runProgressBar()->setProgressColor(QColor());
        m_runPane->runProgressBar()->setText(progressBarMessageFor(Welcome));
    });
    connect(m_runPane->projectsButton(), &PushButton::clicked, this, [=] {
        WindowManager::welcomeWindow()->show();
        WindowManager::welcomeWindow()->raise();
    });
    connect(m_runPane->preferencesButton(), &PushButton::clicked, this, [=] {
        WindowManager::preferencesWindow()->show();
        WindowManager::preferencesWindow()->raise();
    });

    connect(m_runPane->runButton(), &PushButton::clicked,
            this, &RunController::onRunButtonClick);
    connect(RunManager::instance(), &RunManager::processStarted,
            this, &RunController::onProcessStart);
    connect(RunManager::instance(), &RunManager::processFinished,
            this, &RunController::onProcessFinish);
    connect(RunManager::instance(), &RunManager::processErrorOccurred,
            this, &RunController::onProcessErrorOccur);

    connect(m_runPane->stopButton(), &PushButton::clicked, this, &RunController::onStopButtonClick);
    QObject::connect(RunManager::instance(), &RunManager::deviceConnected, this, [=] (const QString& uid) {
        m_runPane->runDevicesButton()->addDevice(RunManager::deviceInfo(uid));
        m_runPane->runDevicesButton()->setCurrentDevice(uid);
    });

    // Aktif hiç proje yüklenmemiş ağ cihazı
    // Aktif proje yükleniyor ağ cihazı
    // Aktif proje çalışıyor ağ cihazı
    // Aktif proje çıkış yapmış ağ cihazı
    //
    // Pasif hiç proje yüklenmemiş ağ cihazı
    // Pasif proje çıkış yapmış ağ cihazı
    //
    // Yerel hiç proje yüklenmemiş cihazı - tanımsız
    // Yerel proje yükleniyor cihazı
    // Yerel proje çalışıyor cihazı
    // Yerel proje çıkış yapmış cihazı    - tanımsız

    QObject::connect(RunManager::instance(), &RunManager::deviceDisconnected, [=] (const QString& uid) {
        if (0) {
            m_runPane->stopButton()->setEnabled(false);

            m_runPane->runProgressBar()->setBusy(false);
            m_runPane->runProgressBar()->setText(progressBarMessageFor(Crashed));
            if (m_runPane->runProgressBar()->progress() == 100) {
                m_runPane->runProgressBar()->setProgressColor(QColor());
                m_runPane->runProgressBar()->setProgress(0);
            } else {
                m_runPane->runProgressBar()->setProgress(100);
                m_runPane->runProgressBar()->setProgressColor("#e05650");
            }
            if (m_runPane->runDevicesButton()->currentDevice() == uid)
                m_runPane->runDevicesButton()->setCurrentDevice(UtilityFunctions::deviceUid(UtilityFunctions::localDeviceInfo()));
            m_runPane->runDevicesButton()->removeDevice(uid);
        } else {
            m_runPane->stopButton()->setEnabled(false);

            m_runPane->runProgressBar()->setBusy(false);
            m_runPane->runProgressBar()->setText(progressBarMessageFor(Crashed));
            if (m_runPane->runProgressBar()->progress() == 100) {
                m_runPane->runProgressBar()->setProgressColor(QColor());
                m_runPane->runProgressBar()->setProgress(0);
            } else {
                m_runPane->runProgressBar()->setProgress(100);
                m_runPane->runProgressBar()->setProgressColor("#e05650");
            }
            if (m_runPane->runDevicesButton()->currentDevice() == uid)
                m_runPane->runDevicesButton()->setCurrentDevice(UtilityFunctions::deviceUid(UtilityFunctions::localDeviceInfo()));
            m_runPane->runDevicesButton()->removeDevice(uid);
        }
    });
    connect(RunManager::instance(), &RunManager::errorOccurred, this, [=] (const QString& errorString) {
        if (RunManager::isLocalDevice(RunManager::recentDevice())) {
            m_runPane->runProgressBar()->setProgressColor("#e05650");
            m_runPane->runProgressBar()->setText(tr(errorString.toUtf8().constData()) + QTime::currentTime().toString());
        }
    });
    connect(RunManager::instance(), &RunManager::uploadProgress, this, [=] (int progress) {
        m_runPane->runProgressBar()->setText(progressBarMessageFor(Running));
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setProgressColor("#e05650");
        m_runPane->runProgressBar()->setBusy(false);
        //        m_runPane->stopButton()->setDisabled(true);
    });
    connect(RunManager::instance(), &RunManager::projectStarted, this, [=] {
        m_runPane->runProgressBar()->setText(progressBarMessageFor(Running));
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setProgressColor("#30acff");
        m_runPane->runProgressBar()->setBusy(false);
    });
    connect(RunManager::instance(), &RunManager::projectFinished, this, [=] (int exitCode) {
        if (exitCode == 0) { // User just closed the app
            m_runPane->runProgressBar()->setText(progressBarMessageFor(Finished));
        } else { // The app has erros thus the interpreter shut itself down
            m_runPane->runProgressBar()->setText(progressBarMessageFor(Crashed));
            m_runPane->runProgressBar()->setProgress(100);
            m_runPane->runProgressBar()->setProgressColor("#e05650");
        }
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->stopButton()->setDisabled(true);
    });
}

void RunController::discharge()
{
    m_appManuallyTerminated = false;
    RunManager::terminate();
    m_runPane->stopButton()->setDisabled(true);
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

void RunController::onRunButtonClick()
{
    m_appManuallyTerminated = false;
    RunManager::execute(m_runPane->runDevicesButton()->currentDevice(), ProjectManager::dir());
    m_runPane->runProgressBar()->setBusy(true);
    m_runPane->runProgressBar()->setProgress(40);
    m_runPane->runProgressBar()->setProgressColor(QColor());
    m_runPane->runProgressBar()->setText(progressBarMessageFor(Starting));
    m_runPane->stopButton()->setEnabled(true);
    emit ran();
}

void RunController::onStopButtonClick()
{
    m_appManuallyTerminated = true;
    RunManager::terminate();
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

QString RunController::progressBarMessageFor(MessageKind kind, const QString& arg)
{
    using namespace UtilityFunctions;
    static const char* welcomeMessage  = QT_TR_NOOP("  :  <b>Ready</b>  |  Welcome to Objectwheel (Beta)");
    static const char* startingMessage = QT_TR_NOOP("  :  <b>Starting</b> the application....");
    static const char* failureMessage  = QT_TR_NOOP("  :  <b>System Failure</b>  |  ");
    static const char* runningMessage  = QT_TR_NOOP("  :  <b>Running</b> on ");
    static const char* crashedMessage  = QT_TR_NOOP("  :  <b>Crashed</b>  |  The application crashed at ");
    static const char* stoppedMessage  = QT_TR_NOOP("  :  <b>Stopped</b>  |  The application terminated at ");
    static const char* finishedMessage = QT_TR_NOOP("  :  <b>Finished</b>  |  The application exited at ");

    const QString& timestamp = QTime::currentTime().toString();
    QString message = "<p style='white-space:pre'>" + ProjectManager::name();

    switch (kind) {
    case Welcome:
        message += tr(welcomeMessage);
        break;
    case Starting:
        message += tr(startingMessage);
        break;
    case Failure:
        message += tr(failureMessage) + arg + ". " + timestamp;
        break;
    case Running:
        message += tr(runningMessage) + deviceName(RunManager::deviceInfo(RunManager::recentDevice()));
        break;
    case Crashed:
        message += tr(crashedMessage) + timestamp;
        break;
    case Stopped:
        message += tr(stoppedMessage) + timestamp;
        break;
    case Finished:
        message += tr(finishedMessage) + timestamp;
        break;
    default:
        break;
    }

    return message += "</p>";
}

void RunController::onProcessFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
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

