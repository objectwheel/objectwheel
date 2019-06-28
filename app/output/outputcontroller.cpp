#include <outputcontroller.h>
#include <outputbar.h>
#include <outputpane.h>
#include <issueswidget.h>
#include <consolewidget.h>
#include <controlpropertymanager.h>
#include <interfacesettings.h>
#include <generalsettings.h>
#include <control.h>
#include <runmanager.h>
#include <projectmanager.h>

#include <QTime>
#include <QAbstractButton>
#include <QStackedWidget>

OutputController::OutputController(OutputPane* outputPane, QObject* parent) : QObject(parent)
  , m_outputPane(outputPane)
{
    connect(m_outputPane->issuesButton(), &QAbstractButton::clicked,
            this, &OutputController::onBarButtonClick);
    connect(m_outputPane->consoleButton(), &QAbstractButton::clicked,
            this, &OutputController::onBarButtonClick);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::imageChanged,
            this, &OutputController::onControlImageChange);
    connect(m_outputPane->issuesWidget(), &IssuesWidget::titleChanged,
            m_outputPane->issuesButton(), &QAbstractButton::setText);
    connect(m_outputPane->issuesWidget(), &IssuesWidget::flash,
            this, &OutputController::onFlash);
    connect(m_outputPane->consoleWidget(), &ConsoleWidget::flash,
            this, &OutputController::onFlash);
    connect(m_outputPane->issuesWidget(), &IssuesWidget::minimized,
            this, &OutputController::onWidgetMinimize);
    connect(m_outputPane->consoleWidget(), &ConsoleWidget::minimized,
            this, &OutputController::onWidgetMinimize);
    //        emit bottomPaneTriggered(m_issuesWidget->isVisible() || m_consoleWidget->isVisible());
    connect(RunManager::instance(), &RunManager::applicationReadyOutput,
            this, &OutputController::onApplicationReadyOutput);
    connect(RunManager::instance(), &RunManager::applicationFinished,
            this, &OutputController::onApplicationFinish);
    connect(RunManager::instance(), &RunManager::applicationErrorOccurred,
            this, &OutputController::onApplicationErrorOccur);
}

void OutputController::discharge()
{
    //    m_consoleWidget->hide();
    //    m_issuesWidget->hide();
    // FIXME   m_outputBar->discharge();
    //    m_consoleWidget->discharge();
    //    m_issuesWidget->discharge();

    //    m_issuesButton->setChecked(false);
    //    m_consoleButton->setChecked(false);

    //    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    //    if (settings->visibleBottomPane != "None") {
    //        if (settings->visibleBottomPane == "Console Pane")
    //            m_consoleButton->click();
    //        else
    //            m_issuesButton->click();
    //    }
}

void OutputController::onBarButtonClick()
{
    auto activatedButton = static_cast<QAbstractButton*>(sender());
    auto activatedWidget = m_outputPane->widgetForButton(activatedButton);

    if (activatedButton->isChecked()) {
        m_outputPane->stackedWidget()->setCurrentWidget(activatedWidget);
        m_outputPane->stackedWidget()->show();
    } else {
        m_outputPane->stackedWidget()->hide();
    }

    if (m_outputPane->issuesButton() != activatedButton)
        m_outputPane->issuesButton()->setChecked(false);
    if (m_outputPane->consoleButton() != activatedButton)
        m_outputPane->consoleButton()->setChecked(false);
}

void OutputController::onWidgetMinimize()
{
    auto activatedWidget = static_cast<QWidget*>(sender());
    auto activatedButton = m_outputPane->buttonForWidget(activatedWidget);
    activatedButton->setChecked(false);
    m_outputPane->stackedWidget()->hide();
}

void OutputController::onFlash()
{
    auto activatedWidget = static_cast<QWidget*>(sender());
    auto activatedButton = m_outputPane->buttonForWidget(activatedWidget);
    const InterfaceSettings* settings = GeneralSettings::interfaceSettings();

    m_outputPane->outputBar()->flash(activatedButton);
    if (settings->bottomPanesPop && !activatedButton->isChecked())
        activatedButton->click();
}

void OutputController::onApplicationReadyOutput(const QString& output)
{
    m_outputPane->consoleWidget()->press(output, m_outputPane->palette().linkVisited());
}

void OutputController::onApplicationFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    const QString& timestamp = QTime::currentTime().toString();
    if (exitStatus != QProcess::CrashExit) {
        if (exitCode == EXIT_FAILURE) {
            m_outputPane->consoleWidget()->press(
                        timestamp + tr(": The application has not exited normally.\n"),
                        QColor("#b34b46"), QFont::DemiBold);
        }
        m_outputPane->consoleWidget()->press(
                    timestamp + ": " + ProjectManager::name() + tr(" exited with code ") +
                    QString::fromUtf8("%1.\n").arg(exitCode), QColor("#025dbf"), QFont::DemiBold);
    }
}

void OutputController::onApplicationErrorOccur(QProcess::ProcessError error, const QString& errorString)
{
    const QString& timestamp = QTime::currentTime().toString();
    if (error == QProcess::FailedToStart) {
        m_outputPane->consoleWidget()->press(
                    timestamp + tr(": System Failure: ") + errorString + "\n",
                    QColor("#b34b46"), QFont::DemiBold);
        m_outputPane->consoleWidget()->press(
                    timestamp + ": " + ProjectManager::name() + tr(" has failed to start.\n"),
                    QColor("#025dbf"), QFont::DemiBold);
    } else {
        m_outputPane->consoleWidget()->press(
                    timestamp + tr(": The application has unexpectedly finished.\n"),
                    QColor("#b34b46"), QFont::DemiBold);
        m_outputPane->consoleWidget()->press(
                    timestamp + ": " + ProjectManager::name() + tr(" has crashed.\n"),
                    QColor("#025dbf"), QFont::DemiBold);
    }
}

void OutputController::onControlImageChange(Control* control, int codeChanged)
{
    if (codeChanged)
        m_outputPane->issuesWidget()->refresh(control);
}
