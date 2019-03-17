#include <runpane.h>
#include <runprogressbar.h>
#include <projectmanager.h>
#include <runmanager.h>
#include <rundevicesbutton.h>
#include <smartspacer.h>
#include <pushbutton.h>
#include <paintutils.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>
#include <generalsettings.h>
#include <interfacesettings.h>

#include <QTime>
#include <QPainter>
#include <QLayout>
#include <QTimer>

using namespace PaintUtils;
using namespace UtilityFunctions;

namespace {
const char* g_welcomeMessage = "<b>Ready</b>  |  Welcome to Objectwheel (Beta)";
const char* g_userStoppedRunningMessage = "<b>Stopped</b>  |  Execution stopped at ";
const char* g_appCrashedMessage = "<b>Crashed</b>  |  Application crashed at ";
const char* g_finishedRunningMessage = "<b>Finished</b>  |  Application closed at ";
const char* g_startRunningMessage = "<b>Starting</b> interpretation...";
const char* g_runningMessage = "<b>Running</b> on ";
}

RunPane::RunPane(QWidget *parent) : QToolBar(parent)
  , m_runButton(new PushButton)
  , m_stopButton(new PushButton)
  , m_runDevicesButton(new RunDevicesButton)
  , m_preferencesButton(new PushButton)
  , m_projectsButton(new PushButton)
  , m_runProgressBar(new RunProgressBar)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    int baseSize = 0;
#if defined(Q_OS_MACOS)
    auto spacer = new QWidget(this);
    spacer->setFixedSize(60, 1);
    baseSize = 67;
    addWidget(spacer);
#endif

    addWidget(m_runButton);
    addWidget(m_stopButton);
    addWidget(m_runDevicesButton);
    addWidget(createSpacerWidget(Qt::Horizontal));
    addWidget(m_runProgressBar);
    addWidget(createSpacerWidget(Qt::Horizontal));
    addWidget(new SmartSpacer(Qt::Horizontal, {m_runDevicesButton}, baseSize, QSize(0, 24),
                              m_runDevicesButton->sizePolicy().horizontalPolicy(),
                              m_runDevicesButton->sizePolicy().verticalPolicy(), this));
    addWidget(m_projectsButton);
    addWidget(m_preferencesButton);

    m_runDevicesButton->setText(tr("Devices"));
    m_runDevicesButton->setIconSize({16, 16});
    m_runDevicesButton->setIcon(QIcon(":/images/devices.png"));
    m_runDevicesButton->setCursor(Qt::PointingHandCursor);

    QObject::connect(RunManager::instance(), &RunManager::deviceConnected,
                     m_runDevicesButton, [=] (const QString& uid) {
        m_runDevicesButton->addDevice(RunManager::deviceInfo(uid));
        m_runDevicesButton->setCurrentDevice(uid);
    });
    QObject::connect(RunManager::instance(), &RunManager::deviceDisconnected, [=] (const QString& uid) {
        if (UtilityFunctions::deviceUid(UtilityFunctions::localDeviceInfo()) == uid)
            return;
        if (m_runDevicesButton->currentDevice() == uid)
            m_runDevicesButton->setCurrentDevice(UtilityFunctions::deviceUid(UtilityFunctions::localDeviceInfo()));
        m_runDevicesButton->removeDevice(uid);
    });

    TransparentStyle::attach(this);
    QTimer::singleShot(200, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(7, 7, 7, 7); // They must be all same
        layout()->setSpacing(7);
    });

    const QColor up = palette().buttonText().color().lighter(130); // not pressed
    const QColor down = palette().buttonText().color().darker(180); // pressed

    QIcon iconRun;
    iconRun.addPixmap(renderMaskedPixmap(":/utils/images/run_small@2x.png", up, this), QIcon::Normal);
    iconRun.addPixmap(renderMaskedPixmap(":/utils/images/run_small@2x.png", down, this), QIcon::Active);
    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setFixedWidth(39);
    m_runButton->setIconSize({16, 16});
    m_runButton->setIcon(iconRun);
    connect(m_runButton, &PushButton::clicked, this, &RunPane::runButtonClicked);
    connect(m_runButton, &PushButton::clicked, this, &RunPane::onRunButtonClick);

    QIcon iconStop;
    iconStop.addPixmap(renderMaskedPixmap(":/utils/images/stop_small@2x.png", up, this), QIcon::Normal);
    iconStop.addPixmap(renderMaskedPixmap(":/utils/images/stop_small@2x.png", down, this), QIcon::Active);
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setFixedWidth(39);
    m_stopButton->setIconSize({16, 16});
    m_stopButton->setIcon(iconStop);
    connect(m_stopButton, &PushButton::clicked, &RunManager::terminate);

    QIcon iconPref;
    iconPref.addPixmap(renderColorizedPixmap(":/images/preferences.png", up, this), QIcon::Normal);
    iconPref.addPixmap(renderColorizedPixmap(":/images/preferences.png", down, this), QIcon::Active);
    m_preferencesButton->setCursor(Qt::PointingHandCursor);
    m_preferencesButton->setToolTip(tr("Show Preferences"));
    m_preferencesButton->setFixedWidth(39);
    m_preferencesButton->setIconSize({16, 16});
    m_preferencesButton->setIcon(iconPref);
    connect(m_preferencesButton, &PushButton::clicked, this, &RunPane::preferencesButtonClicked);

    QIcon iconProj;
    iconProj.addPixmap(renderColorizedPixmap(":/images/projects.png", up, this), QIcon::Normal);
    iconProj.addPixmap(renderColorizedPixmap(":/images/projects.png", down, this), QIcon::Active);
    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setFixedWidth(39);
    m_projectsButton->setIconSize({16, 16});
    m_projectsButton->setIcon(iconProj);
    connect(m_projectsButton, &PushButton::clicked, this, &RunPane::projectsButtonClicked);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] {
        m_runProgressBar->setHtml(tr(g_welcomeMessage));
        m_runProgressBar->setBusy(false);
        m_runProgressBar->setColor(m_runProgressBar->palette().buttonText().color());
    });
    connect(RunManager::instance(), &RunManager::projectStarted,
            this, [=] {
        m_runButton->setEnabled(true);
        m_runProgressBar->setHtml(tr(g_runningMessage) + tr("My Computer"));
        m_runProgressBar->setProgress(100);
        m_runProgressBar->setColor("#30acff");
        m_runProgressBar->setBusy(false);
    });
    connect(RunManager::instance(), &RunManager::projectFinished, this,
            [=] (int exitCode) {
        if (exitCode == 0) // User just closed the app
            m_runProgressBar->setHtml(tr(g_finishedRunningMessage) + QTime::currentTime().toString());
        else { // The app has erros thus the interpreter shut itself down
            m_runProgressBar->setHtml(tr(g_appCrashedMessage) + QTime::currentTime().toString());
            m_runProgressBar->setProgress(100);
            m_runProgressBar->setColor("#e05650");
        }
        m_runProgressBar->setBusy(false);
        m_runButton->setEnabled(true);
        m_stopButton->setDisabled(true);
    });

    connect(RunManager::instance(), &RunManager::deviceDisconnected, this,
            [=] () {
        m_runProgressBar->setHtml(tr(g_appCrashedMessage) + QTime::currentTime().toString());
        m_runProgressBar->setProgress(100);
        m_runProgressBar->setColor("#e05650");
        m_runButton->setEnabled(true);
        m_stopButton->setDisabled(true);
    });
    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged,
            this, qOverload<>(&RunPane::update));
}

void RunPane::discharge()
{
    RunManager::terminate();
    m_runButton->setEnabled(true);
    m_stopButton->setDisabled(true);
}

void RunPane::onRunButtonClick()
{
    RunManager::terminate();

    m_runProgressBar->setBusy(true);
    m_runProgressBar->setProgress(40);
    m_runProgressBar->setHtml(tr(g_startRunningMessage));
    RunManager::execute(m_runDevicesButton->currentDevice(), ProjectManager::dir());

    m_runButton->setDisabled(true);
    m_stopButton->setEnabled(true);
}

void RunPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());
    gradient.setColorAt(0, settings->topBarColor.lighter(110));
    gradient.setColorAt(1, settings->topBarColor.darker(108));
    painter.fillRect(rect(), gradient);
}

QSize RunPane::minimumSizeHint() const
{
    return QSize(0, 38);
}

QSize RunPane::sizeHint() const
{
    return QSize(100, 38);
}
