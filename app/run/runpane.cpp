#include <runpane.h>
#include <pushbutton.h>
#include <rundevicesbutton.h>
#include <runprogressbar.h>
#include <smartspacer.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QLayout>

RunPane::RunPane(QWidget* parent) : QToolBar(parent)
  , m_runButton(new PushButton(this))
  , m_stopButton(new PushButton(this))
  , m_projectsButton(new PushButton(this))
  , m_preferencesButton(new PushButton(this))
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

    int baseSize = 0;
#if defined(Q_OS_MACOS)
    auto spacer = new QWidget(this);
    spacer->setFixedSize(60, 1);
    baseSize = 67;
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
    m_runDevicesButton->setIcon(renderButtonIcon(":/images/devices.png", this));
    m_runButton->setIcon(renderOverlaidButtonIcon(":/images/run.svg", m_runButton));
    m_stopButton->setIcon(renderOverlaidButtonIcon(":/images/stop.svg", m_stopButton));
    m_preferencesButton->setIcon(renderOverlaidButtonIcon(":/images/settings.svg", m_preferencesButton));
    m_projectsButton->setIcon(renderOverlaidButtonIcon(":/images/projects.svg", m_projectsButton));
}

bool RunPane::event(QEvent* event)
{
    // Qt's QToolBar implementation implements a dragging feature on the tool bar
    // when the main window has the unifiedTitleAndToolBarOnMac property enabled
    // On the other hand, macOS already provides its own grabbing and dragging
    // feature on the field where tool bar sits. So they interfere each other
    // We block QToolBar getting mouse events and forward those events to QWidget
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        QWidget::mousePressEvent(static_cast<QMouseEvent*>(event));
        return true;
    case QEvent::MouseButtonRelease:
        QWidget::mouseReleaseEvent(static_cast<QMouseEvent*>(event));
        return true;
    case QEvent::MouseMove:
        QWidget::mouseMoveEvent(static_cast<QMouseEvent*>(event));
        return true;
    default:
        return QToolBar::event(event);
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
    QLinearGradient gradient({0.0, 0.0}, {0.0, 1.0});
    gradient.setCoordinateMode(QGradient::ObjectMode);
    gradient.setColorAt(0, "#e5e5e5");
    gradient.setColorAt(1, "#d7d7d7");
    painter.fillRect(rect(), gradient);
    painter.setPen("#c8c8c8");
    painter.drawLine(QRectF(rect()).bottomLeft() - QPointF(0, 0.5),
                     QRectF(rect()).bottomRight() - QPointF(0, 0.5));
    painter.setPen("#b2b2b2");
    painter.drawLine(QRectF(rect()).bottomLeft(), QRectF(rect()).bottomRight());
    // FIXME: hardcoded, what happens if it is vertical toolbar?
}