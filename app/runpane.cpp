#include <runpane.h>
#include <runpaneloadingbar.h>
#include <windowmanager.h>
#include <projectmanager.h>
#include <runmanager.h>
#include <consolepane.h>
#include <welcomewindow.h>
#include <devicesbutton.h>
#include <smartspacer.h>
#include <pushbutton.h>
#include <paintutils.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>

#include <QTime>
#include <QPainter>
#include <QLayout>
#include <QActionGroup>
#include <QMenu>
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

RunPane::RunPane(ConsolePane* consolePane, QWidget *parent) : QToolBar(parent)
  , m_consolePane(consolePane)
  , m_runButton(new PushButton)
  , m_stopButton(new PushButton)
  , m_devicesButton(new DevicesButton)
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
    addWidget(new SmartSpacer(Qt::Horizontal, {m_devicesButton}, 46, QSize(0, 24),
                              m_devicesButton->sizePolicy().horizontalPolicy(),
                              m_devicesButton->sizePolicy().verticalPolicy(), this));
    addWidget(m_projectsButton);

    m_devicesButton->setCursor(Qt::PointingHandCursor);
    m_devicesButton->setToolTip(tr("Select target device"));

    TransparentStyle::attach(this);
    QTimer::singleShot(200, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(7, 7, 7, 7); // They must be all same
        layout()->setSpacing(7);
    });

    QIcon icon;
    const QColor on = palette().buttonText().color().darker(180); // pressed
    const QColor off = palette().buttonText().color().lighter(130); // not pressed

    icon.addPixmap(renderMaskedPixmap(":/utils/images/run_small@2x.png", on, this), QIcon::Normal, QIcon::On);
    icon.addPixmap(renderMaskedPixmap(":/utils/images/run_small@2x.png", off, this), QIcon::Normal, QIcon::Off);
    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setFixedWidth(39);
    m_runButton->setIconSize({16, 16});
    m_runButton->setIcon(icon);
    connect(m_runButton, SIGNAL(clicked(bool)), SLOT(onRunButtonClick()));

    icon.addPixmap(renderMaskedPixmap(":/utils/images/stop_small@2x.png", on, this), QIcon::Normal, QIcon::On);
    icon.addPixmap(renderMaskedPixmap(":/utils/images/stop_small@2x.png", off, this), QIcon::Normal, QIcon::Off);
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setFixedWidth(39);
    m_stopButton->setIconSize({16, 16});
    m_stopButton->setIcon(icon);
    connect(m_stopButton, SIGNAL(clicked(bool)), SLOT(onStopButtonClick()));

    icon.addPixmap(renderColorizedPixmap(":/images/projects.png", on, this), QIcon::Normal, QIcon::On);
    icon.addPixmap(renderColorizedPixmap(":/images/projects.png", off, this), QIcon::Normal, QIcon::Off);
    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setFixedWidth(39);
    m_projectsButton->setIconSize({16, 16});
    m_projectsButton->setIcon(icon);
    connect(m_projectsButton, SIGNAL(clicked(bool)), SLOT(onProjectsButtonClick()));

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
}

void RunPane::sweep()
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

void RunPane::onStopButtonClick()
{
    RunManager::kill();
}

void RunPane::onRunButtonClick()
{
    m_consolePane->fade();
    if (!m_consolePane->isClean())
        m_consolePane->print("\n");
    m_consolePane->printFormatted(tr("Starting ") + ProjectManager::name() + "...\n", "#025dbf",
                                 QFont::DemiBold);
    m_consolePane->scrollToEnd();

    RunManager::kill();
    RunManager::waitForKill(3000);

    busy(40, tr(g_startRunningMessage));
    RunManager::run();

    m_runButton->setDisabled(true);
    m_stopButton->setEnabled(true);
}

void RunPane::onProjectsButtonClick()
{
    WindowManager::welcomeWindow()->show();
}

void RunPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());
    gradient.setColorAt(0, "#2784E3");
    gradient.setColorAt(1, "#1068C6");
    painter.fillRect(rect(), gradient);
    painter.setPen("#0e5bad");
    painter.drawLine(rect().bottomLeft() + QPointF(0.5, 0.5), rect().bottomRight() + QPointF(0.5, 0.5));
}

QSize RunPane::minimumSizeHint() const
{
    return QSize(0, 38);
}

QSize RunPane::sizeHint() const
{
    return QSize(100, 38);
}
