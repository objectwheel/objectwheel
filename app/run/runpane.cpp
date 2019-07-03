#include <runpane.h>
#include <pushbutton.h>
#include <rundevicesbutton.h>
#include <runprogressbar.h>
#include <segmentedbar.h>
#include <smartspacer.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QLayout>
#include <QPainter>

RunPane::RunPane(QWidget* parent) : QToolBar(parent)
  , m_runButton(new PushButton(this))
  , m_stopButton(new PushButton(this))
  , m_projectsButton(new PushButton(this))
  , m_preferencesButton(new PushButton(this))
  , m_segmentedBar(new SegmentedBar(this))
  , m_runProgressBar(new RunProgressBar(this))
  , m_runDevicesButton(new RunDevicesButton(this))
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Workaround for QToolBarLayout's obsolote serMargin function usage
    QMetaObject::invokeMethod(this, [=] {
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(7, 7, 7, 7); // They must be all same
        layout()->setSpacing(7);
    }, Qt::QueuedConnection);

    m_runProgressBar->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_runDevicesButton->setCursor(Qt::PointingHandCursor);
    m_runDevicesButton->setText(tr("Devices"));
    m_runDevicesButton->setIconSize({16, 16});

    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setFixedWidth(38);
    m_runButton->setIconSize({16, 16});

    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setFixedWidth(38);
    m_stopButton->setIconSize({16, 16});

    m_preferencesButton->setCursor(Qt::PointingHandCursor);
    m_preferencesButton->setToolTip(tr("Show Preferences"));
    m_preferencesButton->setFixedWidth(38);
    m_preferencesButton->setIconSize({16, 16});

    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setFixedWidth(38);
    m_projectsButton->setIconSize({16, 16});

    m_segmentedBar->setCursor(Qt::PointingHandCursor);
    m_segmentedBar->setIconSize({18, 18});

    QAction* leftAction = m_segmentedBar->addAction();
    leftAction->setCheckable(true);
    leftAction->setToolTip(tr("Hide or show left panes"));

    QAction* bottomAction = m_segmentedBar->addAction();
    bottomAction->setCheckable(true);
    bottomAction->setToolTip(tr("Hide or show bottom panes"));

    QAction* rightAction = m_segmentedBar->addAction();
    rightAction->setCheckable(true);
    rightAction->setToolTip(tr("Hide or show right panes"));

    int baseSize = - 7 - m_segmentedBar->sizeHint().width();
#if defined(Q_OS_MACOS)
    auto spacer = new QWidget(this);
    spacer->setFixedSize(63, 1);
    baseSize += 70;
    addWidget(spacer);
#endif

    addWidget(m_runButton);
    addWidget(m_stopButton);
    addWidget(m_runDevicesButton);
    addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    addWidget(m_runProgressBar);
    addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    addWidget(new SmartSpacer(Qt::Horizontal, {m_runDevicesButton}, baseSize, QSize(0, 1),
                              m_runDevicesButton->sizePolicy().horizontalPolicy(),
                              m_runDevicesButton->sizePolicy().verticalPolicy(), this));
    addWidget(m_projectsButton);
    addWidget(m_preferencesButton);
    addWidget(m_segmentedBar);
    updateIcons();
}

PushButton* RunPane::runButton() const
{
    return m_runButton;
}

PushButton* RunPane::stopButton() const
{
    return m_stopButton;
}

PushButton* RunPane::projectsButton() const
{
    return m_projectsButton;
}

PushButton* RunPane::preferencesButton() const
{
    return m_preferencesButton;
}

SegmentedBar* RunPane::segmentedBar() const
{
    return m_segmentedBar;
}

RunProgressBar* RunPane::runProgressBar() const
{
    return m_runProgressBar;
}

RunDevicesButton* RunPane::runDevicesButton() const
{
    return m_runDevicesButton;
}

QSize RunPane::sizeHint() const
{
    return {100, 38};
}

QSize RunPane::minimumSizeHint() const
{
    return {0, 38};
}

void RunPane::updateIcons()
{
    using namespace PaintUtils;
    m_runDevicesButton->setIcon(QIcon(":/images/devices.png"));
    m_runButton->setIcon(renderButtonIcon(":/images/run.svg", palette()));
    m_stopButton->setIcon(renderButtonIcon(":/images/stop.svg", palette()));
    m_preferencesButton->setIcon(renderButtonIcon(":/images/settings.svg", palette()));
    m_projectsButton->setIcon(renderButtonIcon(":/images/projects.svg", palette()));

    QStringList iconFileNames {
        ":/images/left.svg",
        ":/images/bottom.svg",
        ":/images/right.svg"
    };
    for (int i = 0; i < iconFileNames.size(); ++i) {
        QAction* action = m_segmentedBar->actions().at(i);
        action->setIcon(renderButtonIcon(iconFileNames.at(i), palette()));
    }
}

void RunPane::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        updateIcons();
    }
    QWidget::changeEvent(event);
}

void RunPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, "#e5e5e5");
    gradient.setColorAt(1, "#d6d6d6");
    painter.fillRect(rect(), gradient);
    painter.setPen("#c7c7c7");
    painter.drawLine(QRectF(rect()).bottomLeft() - QPointF(0, 0.5),
                     QRectF(rect()).bottomRight() - QPointF(0, 0.5));
    painter.setPen("#b1b1b1");
    painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
    // FIXME: hardcoded, what happens if it is vertical toolbar?
}