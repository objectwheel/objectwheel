#include <buildspane.h>
#include <segmentedbar.h>
#include <applicationstyle.h>
#include <utilityfunctions.h>
#include <androidplatformwidget.h>

#include <QStackedWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QAction>

BuildsPane::BuildsPane(QWidget* parent) : QWidget(parent)
  , m_segmentedBar(new SegmentedBar(this))
  , m_stackedWidget(new QStackedWidget(this))
  , m_androidWidget(new AndroidPlatformWidget(this))
{
    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_stackedWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(3, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->addWidget(m_segmentedBar, 1, 1);
    layout->addWidget(scrollArea, 2, 1);

    m_stackedWidget->addWidget(m_androidWidget);
    m_stackedWidget->setCurrentWidget(m_androidWidget);

    ApplicationStyle::setButtonStyle(m_segmentedBar, ApplicationStyle::TexturedRounded);
    ApplicationStyle::setHighlightingDisabledForCheckedState(m_segmentedBar, true);
    m_segmentedBar->setCursor(Qt::PointingHandCursor);
    m_segmentedBar->setIconSize({18, 18});

    QAction* leftAction = m_segmentedBar->addAction();
    leftAction->setCheckable(true);
    leftAction->setShortcut(Qt::CTRL + Qt::Key_J);
    leftAction->setToolTip(tr("Hide or show left panes") + UtilityFunctions::shortcutSymbol(leftAction->shortcut()));

    QAction* bottomAction = m_segmentedBar->addAction();
    bottomAction->setCheckable(true);
    bottomAction->setShortcut(Qt::CTRL + Qt::Key_K);
    bottomAction->setToolTip(tr("Hide or show bottom panes") + UtilityFunctions::shortcutSymbol(bottomAction->shortcut()));

    QAction* rightAction = m_segmentedBar->addAction();
    rightAction->setCheckable(true);
    rightAction->setShortcut(Qt::CTRL + Qt::Key_L);
    rightAction->setToolTip(tr("Hide or show right panes") + UtilityFunctions::shortcutSymbol(rightAction->shortcut()));
}

SegmentedBar* BuildsPane::segmentedBar() const
{
    return m_segmentedBar;
}

QStackedWidget* BuildsPane::stackedWidget() const
{
    return m_stackedWidget;
}

AndroidPlatformWidget* BuildsPane::androidWidget() const
{
    return m_androidWidget;
}

QSize BuildsPane::sizeHint() const
{
    return {670, 640};
}

QSize BuildsPane::minimumSizeHint() const
{
    return {0, 0};
}
