#include <outputcontroller.h>
#include <outputbar.h>
#include <outputpane.h>
#include <issueswidget.h>
#include <consolewidget.h>
#include <controlpropertymanager.h>
#include <control.h>
#include <generalsettings.h>
#include <interfacesettings.h>
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
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
            this, &OutputController::onControlRenderInfoChange);
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
    connect(RunManager::instance(), &RunManager::applicationReadyOutput,
            this, &OutputController::onApplicationReadyOutput);
    connect(RunManager::instance(), &RunManager::applicationFinished,
            this, &OutputController::onApplicationFinish);
    connect(RunManager::instance(), &RunManager::applicationErrorOccurred,
            this, &OutputController::onApplicationErrorOccur);
    connect(GeneralSettings::instance(), &GeneralSettings::designerStateReset,
            this, [=] {
        InterfaceSettings* settings = GeneralSettings::interfaceSettings();
        settings->begin();
        settings->setValue("OutputPane.CurrentWidget", 0);
        settings->end();
        m_outputPane->stackedWidget()->setCurrentWidget(m_outputPane->widgetForButton(m_outputPane->outputBar()->buttons().at(0)));
    });

    setCurrentWidget(nullptr);

    InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    settings->begin();
    const int index = settings->value<int>("OutputPane.CurrentWidget", 0);
    settings->end();
    if (m_outputPane->outputBar()->buttons().size() > index && index >= 0)
        m_outputPane->stackedWidget()->setCurrentWidget(m_outputPane->widgetForButton(m_outputPane->outputBar()->buttons().at(index)));
}

void OutputController::discharge()
{
    m_outputPane->issuesWidget()->clean();
    m_outputPane->consoleWidget()->clear();
}

void OutputController::setPaneVisible(bool visible)
{
    if (visible)
        setCurrentWidget(m_outputPane->stackedWidget()->currentWidget());
    else
        setCurrentWidget(nullptr);
}

void OutputController::onFlash()
{
    if (auto activatedWidget = static_cast<QWidget*>(sender())) {
        if (QAbstractButton* activatedButton = m_outputPane->buttonForWidget(activatedWidget)) {
            m_outputPane->outputBar()->flash(activatedButton);
            const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
            if (settings->outputPanePops && !activatedButton->isChecked())
                setCurrentWidget(activatedWidget);
        }
    }
}

void OutputController::onBarButtonClick()
{
    if (auto activatedButton = static_cast<QAbstractButton*>(sender())) {
        if (activatedButton->isChecked())
            setCurrentWidget(m_outputPane->widgetForButton(activatedButton));
        else
            setCurrentWidget(nullptr);
    }
}

void OutputController::onWidgetMinimize()
{
    setCurrentWidget(nullptr);
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

void OutputController::onControlRenderInfoChange(Control* control, int codeChanged)
{
    if (codeChanged)
        m_outputPane->issuesWidget()->refresh(control);
}

void OutputController::setCurrentWidget(QWidget* widget)
{
    for (QAbstractButton* button : m_outputPane->outputBar()->buttons())
        button->setChecked(false);

    if (widget == 0) {
        m_outputPane->stackedWidget()->hide();
        emit currentWidgetChanged(nullptr);
        return;
    }

    if (QAbstractButton* button = m_outputPane->buttonForWidget(widget)) {
        button->setChecked(true);
        m_outputPane->setUpdatesEnabled(false);
        m_outputPane->stackedWidget()->setCurrentWidget(widget);
        m_outputPane->stackedWidget()->show();
        m_outputPane->setUpdatesEnabled(true);

        if (GeneralSettings::interfaceSettings()->preserveDesignerState) {
            InterfaceSettings* settings = GeneralSettings::interfaceSettings();
            settings->begin();
            for (int i = 0; i < m_outputPane->outputBar()->buttons().size(); ++i) {
                if (m_outputPane->outputBar()->buttons().at(i) == button) {
                    settings->setValue("OutputPane.CurrentWidget", i);
                    break;
                }
            }
            settings->end();
        }

        emit currentWidgetChanged(widget);
    }
}
