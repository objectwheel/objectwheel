#include <buildspane.h>
#include <segmentedbar.h>
#include <genericplatformwidget.h>
#include <androidplatformwidget.h>
#include <applicationstyle.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QStackedWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QAction>
#include <QLabel>

BuildsPane::BuildsPane(QWidget* parent) : QWidget(parent)
  , m_platformLabel(new QLabel(this))
  , m_segmentedBar(new SegmentedBar(this))
  , m_stackedWidget(new QStackedWidget(this))
  , m_genericWidget(new GenericPlatformWidget(this))
  , m_androidWidget(new AndroidPlatformWidget(this))
{
    auto iconLabel = new QLabel(this);
    auto titleLabel = new QLabel(tr("Objectwheel Cloud Builds"), this);
    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_stackedWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(8, 1);
    layout->addWidget(iconLabel, 1, 1, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 2, 1, Qt::AlignHCenter);
    layout->addWidget(m_platformLabel, 3, 1, Qt::AlignHCenter);
    layout->setRowMinimumHeight(4, 8);
    layout->addWidget(scrollArea, 5, 1);
    layout->setRowMinimumHeight(6, 8);
    layout->addWidget(m_segmentedBar, 7, 1);

    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/builds/gift.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(26);
    titleLabel->setFont(f);

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_platformLabel->setFont(f);

    m_stackedWidget->addWidget(m_genericWidget);
    m_stackedWidget->addWidget(m_androidWidget);
    m_stackedWidget->setCurrentWidget(m_genericWidget);

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
