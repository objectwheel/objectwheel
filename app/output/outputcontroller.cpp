#include <outputcontroller.h>
#include <outputpane.h>
#include <QAbstractButton>
#include <QStackedWidget>

OutputController::OutputController(OutputPane* outputPane, QObject* parent) : QObject(parent)
  , m_outputPane(outputPane)
{
    connect(m_outputPane->issuesButton(), &QAbstractButton::clicked,
            this, &OutputController::onBarButtonClick);
    connect(m_outputPane->consoleButton(), &QAbstractButton::clicked,
            this, &OutputController::onBarButtonClick);

    //    connect(ControlPropertyManager::instance(), &ControlPropertyManager::imageChanged,
    //            this, [=] (Control* control, int codeChanged) {
    //        if (codeChanged)
    //            m_issuesWidget->refresh(control);
    //    });
    //    connect(m_issuesWidget, &IssuesWidget::titleChanged,
    //            m_outputBar->issuesButton(), &QAbstractButton::setText);
    //    connect(m_issuesWidget, &IssuesWidget::designsFileOpened,
    //            m_designerWidget, &DesignerWidget::onDesignsFileOpen);
    //    connect(m_issuesWidget, &IssuesWidget::assetsFileOpened,
    //            m_designerWidget, &DesignerWidget::onAssetsFileOpen);
    //    connect(m_consoleWidget, &ConsoleWidget::designsFileOpened,
    //            m_designerWidget, &DesignerWidget::onDesignsFileOpen);
    //    connect(m_consoleWidget, &ConsoleWidget::assetsFileOpened,
    //            m_designerWidget, &DesignerWidget::onAssetsFileOpen);
    //    connect(m_issuesWidget, &IssuesWidget::flash,
    //            this, [=] {
    // FIXME       void OutputBar::flash(QAbstractButton* button)
    //        {
    //            if (button == m_consoleButton)
    //                m_consoleFlasher->flash(400, 3);
    //            else if (button == m_issuesButton)
    //                m_issuesFlasher->flash(400, 3);

    //            const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
    //            if (settings->bottomPanesPop && !button->isChecked())
    //                button->click();
    //        }
    //        m_outputBar->flash(m_outputBar->issuesButton());
    //    });
    //    connect(m_consoleWidget, &ConsoleWidget::flash,
    //            this, [=] {
    //        m_outputBar->flash(m_outputBar->consoleButton());
    //    });
    //    connect(m_issuesWidget, &IssuesWidget::minimized,
    //            this, [=] {
    //        m_outputBar->issuesButton()->setChecked(false);
    //        m_issuesWidget->hide();
    //        emit bottomPaneTriggered(false);
    //    });
    //    connect(m_consoleWidget, &ConsoleWidget::minimized,
    //            this, [=] {
    //        m_outputBar->consoleButton()->setChecked(false);
    //        m_consoleWidget->hide();
    //        emit bottomPaneTriggered(false);
    //    });
    //    connect(m_outputBar, &OutputBar::buttonActivated,
    //            this, [=] (QAbstractButton* button) {
    //        if (button == m_outputBar->consoleButton()) {
    //            if (button->isChecked())
    //                m_consoleWidget->show();
    //            else
    //                m_consoleWidget->hide();
    //        } else {
    //            if (button->isChecked())
    //                m_issuesWidget->show();
    //            else
    //                m_issuesWidget->hide();
    //        }
    //        emit bottomPaneTriggered(m_issuesWidget->isVisible() || m_consoleWidget->isVisible());
    //    });

    //    connect(RunManager::instance(), &RunManager::applicationReadyOutput, this, [=] (const QString& output)
    //    { m_centralWidget->consoleWidget()->press(output, palette().linkVisited()); });
    //    connect(RunManager::instance(), &RunManager::applicationFinished,
    //            [=] (int exitCode, QProcess::ExitStatus exitStatus) {
    //        auto console = m_centralWidget->consoleWidget();
    //        auto timestamp = QTime::currentTime().toString();
    //        if (exitStatus != QProcess::CrashExit) {
    //            if (exitCode == EXIT_FAILURE) {
    //                console->press(timestamp + tr(": The application has not exited normally.\n"),
    //                               QColor("#b34b46"), QFont::DemiBold);
    //            }
    //            console->press(timestamp + ": " + ProjectManager::name() + tr(" exited with code ") +
    //                           QString::fromUtf8("%1.\n").arg(exitCode), QColor("#025dbf"), QFont::DemiBold);
    //        }
    //    });
    //    connect(RunManager::instance(), &RunManager::applicationErrorOccurred,
    //            [=] (QProcess::ProcessError error, const QString& errorString) {
    //        auto console = m_centralWidget->consoleWidget();
    //        auto timestamp = QTime::currentTime().toString();
    //        if (error == QProcess::FailedToStart) {
    //            console->press(timestamp + tr(": System Failure: ") + errorString + "\n",
    //                           QColor("#b34b46"), QFont::DemiBold);
    //            console->press(timestamp + ": " + ProjectManager::name() + tr(" has failed to start.\n"),
    //                           QColor("#025dbf"), QFont::DemiBold);
    //        } else {
    //            console->press(timestamp + tr(": The application has unexpectedly finished.\n"),
    //                           QColor("#b34b46"), QFont::DemiBold);
    //            console->press(timestamp + ": " + ProjectManager::name() + tr(" has crashed.\n"),
    //                           QColor("#025dbf"), QFont::DemiBold);
    //        }
    //    });

    //    connect(m_runController, &RunController::ran, this, [=] {
    //        auto timestamp = QTime::currentTime().toString();
    //        BehaviorSettings* settings = CodeEditorSettings::behaviorSettings();
    //        if (settings->autoSaveBeforeRunning)
    //            WindowManager::mainWindow()->centralWidget()->qmlCodeEditorWidget()->saveAll();
    //        m_centralWidget->consoleWidget()->fade();
    //        if (!m_centralWidget->consoleWidget()->toPlainText().isEmpty())
    //            m_centralWidget->consoleWidget()->press("\n");
    //        m_centralWidget->consoleWidget()->press(timestamp + tr(": Starting") + " " + ProjectManager::name() + "...\n",
    //                                              QColor("#025dbf"), QFont::DemiBold);
    //        m_centralWidget->consoleWidget()->verticalScrollBar()->
    //                setValue(m_centralWidget->consoleWidget()->verticalScrollBar()->maximum());
    //    });

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

    if (activatedButton->isChecked())
        m_outputPane->stackedWidget()->setCurrentWidget(m_outputPane->widgetForButton(activatedButton));
    else
        m_outputPane->stackedWidget()->hide();

    if (m_outputPane->issuesButton() != activatedButton) {
        m_outputPane->issuesButton()->setChecked(false);
    }

    if (m_outputPane->consoleButton() != activatedButton) {
        m_outputPane->consoleButton()->setChecked(false);
    }
}
