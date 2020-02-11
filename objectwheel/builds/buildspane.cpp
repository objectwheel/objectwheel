#include <buildspane.h>
#include <segmentedbar.h>
#include <startwidget.h>
#include <androidplatformwidget.h>
#include <androidplatformcontroller.h>
#include <applicationstyle.h>

#include <QStackedWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QAction>
#include <QLabel>

BuildsPane::BuildsPane(QWidget* parent) : QScrollArea(parent)
  , m_platformLabel(new QLabel(this))
  , m_segmentedBar(new SegmentedBar(this))
  , m_stackedWidget(new QStackedWidget(this))
  , m_startWidget(new StartWidget(this))
  , m_androidPlatformWidget(new AndroidPlatformWidget(this))
  , m_androidPlatformController(new AndroidPlatformController(m_androidPlatformWidget, this))
{
    setWidgetResizable(true);
    setWidget(new QWidget(this));
    setFrameShape(QFrame::NoFrame);
    auto layout = new QGridLayout(widget());
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->addWidget(m_segmentedBar, 0, 1, Qt::AlignHCenter);
    layout->addWidget(m_platformLabel, 1, 1, Qt::AlignHCenter);
    layout->addWidget(m_stackedWidget, 4, 1);

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_platformLabel->setFont(f);

    m_stackedWidget->addWidget(m_startWidget);
    m_stackedWidget->addWidget(m_androidPlatformWidget);
    m_stackedWidget->setCurrentWidget(m_startWidget);

    ApplicationStyle::setButtonStyle(m_segmentedBar, ApplicationStyle::TexturedRounded);
    ApplicationStyle::setHighlightingDisabledForCheckedState(m_segmentedBar, true);
    m_segmentedBar->setCursor(Qt::PointingHandCursor);
    m_segmentedBar->setIconSize({18, 18});

    QAction* startAction = m_segmentedBar->addAction(tr("Start"));
    startAction->setCheckable(true);
    startAction->setShortcut(Qt::CTRL + Qt::Key_J);
    startAction->setToolTip(tr("Hide or show left panes"));

    QAction* settingsAction = m_segmentedBar->addAction(tr("Settings"));
    settingsAction->setCheckable(true);
    settingsAction->setShortcut(Qt::CTRL + Qt::Key_L);
    settingsAction->setToolTip(tr("Hide or show right panes"));

    QAction* downloadsAction = m_segmentedBar->addAction(tr("Downloads"));
    downloadsAction->setCheckable(true);
    downloadsAction->setShortcut(Qt::CTRL + Qt::Key_L);
    downloadsAction->setToolTip(tr("Hide or show right panes"));
}

QLabel* BuildsPane::platformLabel() const
{
    return m_platformLabel;
}

SegmentedBar* BuildsPane::segmentedBar() const
{
    return m_segmentedBar;
}

QStackedWidget* BuildsPane::stackedWidget() const
{
    return m_stackedWidget;
}

StartWidget* BuildsPane::startWidget() const
{
    return m_startWidget;
}

AndroidPlatformWidget* BuildsPane::androidPlatformWidget() const
{
    return m_androidPlatformWidget;
}

void BuildsPane::charge() const
{
    m_androidPlatformController->charge();
}

void BuildsPane::discharge() const
{
    m_androidPlatformController->discharge();
}

QSize BuildsPane::sizeHint() const
{
    return {670, 640};
}

QSize BuildsPane::minimumSizeHint() const
{
    return {0, 0};
}
