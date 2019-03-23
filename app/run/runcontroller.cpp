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

namespace {
const char* g_welcomeMessage = "<p style='white-space:pre'><b>Ready</b>  | asdasd h hh hhhh- asd sda akkk</p>";
const char* g_userStoppedRunningMessage = "<b>Stopped</b>  |  Execution stopped at ";
const char* g_appCrashedMessage = "<b>Crashed</b>  |  Application crashed at ";
const char* g_finishedRunningMessage = "<b>Finished</b>  |  Application closed at ";
const char* g_startRunningMessage = "<b>Starting</b> interpretation...";
const char* g_runningMessage = "<b>Running</b> on ";
}

RunController::RunController(RunPane* runPane, QObject* parent) : QObject(parent)
  , m_runPane(runPane)
{    
    onInterfaceSettingsChange();

    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged,
            this, &RunController::onInterfaceSettingsChange);
    connect(ProjectManager::instance(), &ProjectManager::started, this, [=] {
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->runProgressBar()->setProgress(0);
        m_runPane->runProgressBar()->setProgressColor(QColor());
        m_runPane->runProgressBar()->setText(tr(g_welcomeMessage));
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

    connect(m_runPane->stopButton(), &PushButton::clicked, &RunManager::terminate);
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
            m_runPane->runProgressBar()->setText(tr(g_appCrashedMessage) + QTime::currentTime().toString());
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
            m_runPane->runProgressBar()->setText(tr(g_appCrashedMessage) + QTime::currentTime().toString());
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
        m_runPane->runProgressBar()->setText(tr(g_runningMessage) + QTime::currentTime().toString());
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setProgressColor("#e05650");
        m_runPane->runProgressBar()->setBusy(false);
        //        m_runPane->stopButton()->setDisabled(true);
    });
    connect(RunManager::instance(), &RunManager::projectStarted, this, [=] {
        m_runPane->runProgressBar()->setText(tr(g_runningMessage) + tr("My Computer"));
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setProgressColor("#30acff");
        m_runPane->runProgressBar()->setBusy(false);
    });
    connect(RunManager::instance(), &RunManager::projectFinished, this, [=] (int exitCode) {
        if (exitCode == 0) { // User just closed the app
            m_runPane->runProgressBar()->setText(tr(g_finishedRunningMessage) + QTime::currentTime().toString());
        } else { // The app has erros thus the interpreter shut itself down
            m_runPane->runProgressBar()->setText(tr(g_appCrashedMessage) + QTime::currentTime().toString());
            m_runPane->runProgressBar()->setProgress(100);
            m_runPane->runProgressBar()->setProgressColor("#e05650");
        }
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->stopButton()->setDisabled(true);
    });
}

void RunController::discharge()
{
    RunManager::terminate();
    m_runPane->stopButton()->setDisabled(true);
}

void RunController::onInterfaceSettingsChange()
{
    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    QPalette palette(m_runPane->palette());
    QLinearGradient gradient({0.0, 0.0}, {0.0, 1.0});
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, settings->topBarColor.lighter(110));
    gradient.setColorAt(1, settings->topBarColor.darker(108));
    palette.setBrush(QPalette::Window, gradient);
    m_runPane->setPalette(palette);
}

void RunController::onRunButtonClick()
{
    RunManager::execute(m_runPane->runDevicesButton()->currentDevice(), ProjectManager::dir());
    m_runPane->runProgressBar()->setBusy(true);
    m_runPane->runProgressBar()->setProgress(0);
    m_runPane->runProgressBar()->setProgressColor(QColor());
    m_runPane->runProgressBar()->setText(tr(g_startRunningMessage));
    m_runPane->stopButton()->setEnabled(true);
    emit ran();
}

void RunController::onProcessStart()
{
    m_runPane->runProgressBar()->setBusy(false);
    m_runPane->runProgressBar()->setProgress(100);
    m_runPane->runProgressBar()->setProgressColor("#30acff");
    m_runPane->runProgressBar()->setText(tr(g_runningMessage) + tr("My Computer"));
}

void RunController::onProcessErrorOccur(QProcess::ProcessError error, const QString& errorString)
{
    if (error == QProcess::FailedToStart) {
        m_runPane->stopButton()->setEnabled(false);
        m_runPane->runProgressBar()->setBusy(false);
        m_runPane->runProgressBar()->setProgress(100);
        m_runPane->runProgressBar()->setProgressColor("#e05650");
        m_runPane->runProgressBar()->setText(tr(errorString.toUtf8().constData()));
    } else {
        m_runPane->runProgressBar()->setText(tr(errorString.toUtf8().constData()));
    }
}

void RunController::onProcessFinish(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::CrashExit)
        m_runPane->runProgressBar()->setText(tr(g_finishedRunningMessage) + tr("My Computer"));
    m_runPane->stopButton()->setEnabled(false);
}

