#include <outputpane.h>
#include <issueswidget.h>
#include <consolewidget.h>
#include <outputbar.h>
#include <paintutils.h>

#include <QStackedWidget>
#include <QBoxLayout>
#include <QAbstractButton>

OutputPane::OutputPane(QWidget* parent) : QWidget(parent)
  , m_stackedWidget(new QStackedWidget)
  , m_issuesWidget(new IssuesWidget(this))
  , m_consoleWidget(new ConsoleWidget(this))
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto outputBar = new OutputBar(this);
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stackedWidget);
    layout->addWidget(outputBar);

    m_stackedWidget->layout()->setSpacing(0);
    m_stackedWidget->layout()->setContentsMargins(0, 0, 0, 0);
    m_stackedWidget->addWidget(m_issuesWidget);
    m_stackedWidget->addWidget(m_consoleWidget);

    m_issuesButton = outputBar->addButton();
    m_issuesButton->setCheckable(true);
    m_issuesButton->setFixedHeight(22);
    m_issuesButton->setCursor(Qt::PointingHandCursor);
    m_issuesButton->setText(tr("Issues") + " [0]");
    m_issuesButton->setToolTip(tr("Activate issues list"));
    m_issuesButton->setIconSize({14, 14});
    m_issuesButton->setIcon(PaintUtils::renderButtonIcon(":/images/issues.svg",
                                                       m_issuesButton->palette()));

    m_consoleButton = outputBar->addButton();
    m_consoleButton->setCheckable(true);
    m_consoleButton->setFixedHeight(22);
    m_consoleButton->setCursor(Qt::PointingHandCursor);
    m_consoleButton->setText(tr("Console"));
    m_consoleButton->setToolTip(tr("Activate console output"));
    m_consoleButton->setIconSize({14, 14});
    m_consoleButton->setIcon(PaintUtils::renderButtonIcon(":/images/console.svg",
                                                          m_consoleButton->palette()));
}

QStackedWidget* OutputPane::stackedWidget() const
{
    return m_stackedWidget;
}

QAbstractButton* OutputPane::issuesButton() const
{
    return m_issuesButton;
}

QAbstractButton* OutputPane::consoleButton() const
{
    return m_consoleButton;
}

IssuesWidget* OutputPane::issuesWidget() const
{
    return m_issuesWidget;
}

ConsoleWidget* OutputPane::consoleWidget() const
{
    return m_consoleWidget;
}

QWidget* OutputPane::widgetForButton(QAbstractButton* button) const
{
    if (button == m_issuesButton)
        return m_issuesWidget;
    return m_consoleWidget;
}
