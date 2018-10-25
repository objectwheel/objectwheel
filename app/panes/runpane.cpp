#include <runpane.h>
#include <runpaneloadingbar.h>
#include <projectmanager.h>
#include <runmanager.h>
#include <devicesbutton.h>
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
const char* g_welcomeMessage = "<b>Ready</b>  |  Welcome to Objectwheel";
const char* g_userStoppedRunningMessage = "<b>Stopped</b>  |  Execution stopped at ";
const char* g_appCrashedMessage = "<b>Crashed</b>  |  Application crashed at ";
const char* g_finishedRunningMessage = "<b>Finished</b>  |  Application closed at ";
const char* g_startRunningMessage = "<b>Starting</b> interpretation...";
const char* g_runningMessage = "<b>Running</b> on ";
}

RunPane::RunPane(QWidget *parent) : QToolBar(parent)
  , m_runButton(new PushButton)
  , m_stopButton(new PushButton)
  , m_devicesButton(new DevicesButton)
  , m_preferencesButton(new PushButton)
  , m_projectsButton(new PushButton)
  , m_loadingBar(new RunPaneLoadingBar)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    addWidget(m_runButton);
    addWidget(m_stopButton);
    addWidget(m_devicesButton);
    addWidget(createSpacerWidget(Qt::Horizontal));
    addWidget(m_loadingBar);
    addWidget(createSpacerWidget(Qt::Horizontal));
    addWidget(new SmartSpacer(Qt::Horizontal, {m_devicesButton}, 0, QSize(0, 24),
                              m_devicesButton->sizePolicy().horizontalPolicy(),
                              m_devicesButton->sizePolicy().verticalPolicy(), this));
    addWidget(m_projectsButton);
    addWidget(m_preferencesButton);

    m_devicesButton->setCursor(Qt::PointingHandCursor);
    m_devicesButton->setToolTip(tr("Select target device"));

    TransparentStyle::attach(this);
    QTimer::singleShot(200, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(7, 7, 7, 7); // They must be all same
        layout()->setSpacing(7);
    });

    const QColor on = palette().buttonText().color().darker(180); // pressed
    const QColor off = palette().buttonText().color().lighter(130); // not pressed

    QIcon iconRun;
    iconRun.addPixmap(renderMaskedPixmap(":/utils/images/run_small@2x.png", on, this), QIcon::Normal, QIcon::On);
    iconRun.addPixmap(renderMaskedPixmap(":/utils/images/run_small@2x.png", off, this), QIcon::Normal, QIcon::Off);
    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setFixedWidth(39);
    m_runButton->setIconSize({16, 16});
    m_runButton->setIcon(iconRun);
    connect(m_runButton, &PushButton::clicked, this, &RunPane::runButtonClicked);
    connect(m_runButton, &PushButton::clicked, this, &RunPane::onRunButtonClick);

    QIcon iconStop;
    iconStop.addPixmap(renderMaskedPixmap(":/utils/images/stop_small@2x.png", on, this), QIcon::Normal, QIcon::On);
    iconStop.addPixmap(renderMaskedPixmap(":/utils/images/stop_small@2x.png", off, this), QIcon::Normal, QIcon::Off);
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setFixedWidth(39);
    m_stopButton->setIconSize({16, 16});
    m_stopButton->setIcon(iconStop);
    connect(m_stopButton, &PushButton::clicked, &RunManager::kill);

    QIcon iconPref;
    iconPref.addPixmap(renderColorizedPixmap(":/images/preferences.png", on, this), QIcon::Normal, QIcon::On);
    iconPref.addPixmap(renderColorizedPixmap(":/images/preferences.png", off, this), QIcon::Normal, QIcon::Off);
    m_preferencesButton->setCursor(Qt::PointingHandCursor);
    m_preferencesButton->setToolTip(tr("Show Preferences"));
    m_preferencesButton->setFixedWidth(39);
    m_preferencesButton->setIconSize({16, 16});
    m_preferencesButton->setIcon(iconPref);
    connect(m_preferencesButton, &PushButton::clicked, this, &RunPane::preferencesButtonClicked);

    QIcon iconProj;
    iconProj.addPixmap(renderColorizedPixmap(":/images/projects.png", on, this), QIcon::Normal, QIcon::On);
    iconProj.addPixmap(renderColorizedPixmap(":/images/projects.png", off, this), QIcon::Normal, QIcon::Off);
    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setFixedWidth(39);
    m_projectsButton->setIconSize({16, 16});
    m_projectsButton->setIcon(iconProj);
    connect(m_projectsButton, &PushButton::clicked, this, &RunPane::projectsButtonClicked);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] { setMessage(tr(g_welcomeMessage)); });
    connect(RunManager::instance(), &RunManager::started, this,
            [=] {
        m_runButton->setEnabled(true);
        done(tr(g_runningMessage) + tr("My Computer")); // TODO: Fix this "My Computer" thing
    });
    connect(RunManager::instance(), &RunManager::finished, this,
            [=] (int exitCode, QProcess::ExitStatus status) {
        if (status == QProcess::CrashExit) // Stopped by user
            setMessage(tr(g_userStoppedRunningMessage) + QTime::currentTime().toString());
        else if (exitCode == 0) // User just closed the app
            setMessage(tr(g_finishedRunningMessage) + QTime::currentTime().toString());
        else // The app has erros thus the interpreter shut itself down
            error(tr(g_appCrashedMessage) + QTime::currentTime().toString());
        m_runButton->setEnabled(true);
        m_stopButton->setDisabled(true);
    });
    connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged,
            this, qOverload<>(&RunPane::update));
}

void RunPane::discharge()
{
    RunManager::kill();
    m_runButton->setEnabled(true);
    m_stopButton->setDisabled(true);
}

void RunPane::busy(int progress, const QString& message)
{
    m_loadingBar->busy(progress, ProjectManager::name() + ": " + message);
}

void RunPane::done(const QString& message)
{
    m_loadingBar->done(ProjectManager::name() + ": " + message);
}

void RunPane::error(const QString& message)
{
    m_loadingBar->error(ProjectManager::name() + ": " + message);
}

void RunPane::setMessage(const QString& message)
{
    m_loadingBar->setText(ProjectManager::name() + ": " + message);
}

void RunPane::onRunButtonClick()
{
    RunManager::kill();
    RunManager::waitForKill(3000);

    busy(40, tr(g_startRunningMessage));
    RunManager::run();

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
