#include <runpane.h>
#include <rundevicesbutton.h>
#include <runprogressbar.h>
#include <segmentedbar.h>
#include <smartspacer.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <applicationstyle.h>

#include <QPushButton>
#include <QLayout>
#include <QPainter>

RunPane::RunPane(QWidget* parent) : QToolBar(parent)
  , m_runButton(new QPushButton(this))
  , m_stopButton(new QPushButton(this))
  , m_projectsButton(new QPushButton(this))
  , m_preferencesButton(new QPushButton(this))
  , m_segmentedBar(new SegmentedBar(this))
  , m_runProgressBar(new RunProgressBar(this))
  , m_runDevicesButton(new RunDevicesButton(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    layout()->setSpacing(7);
    layout()->setContentsMargins(7, 7, 7, 7);

    ApplicationStyle::setButtonStyle(m_runProgressBar, ApplicationStyle::TexturedRounded);
    m_runProgressBar->setAttribute(Qt::WA_TransparentForMouseEvents);

    ApplicationStyle::setButtonStyle(m_runDevicesButton, ApplicationStyle::TexturedRounded);
    m_runDevicesButton->setCursor(Qt::PointingHandCursor);
    m_runDevicesButton->setText(tr("Devices"));

    ApplicationStyle::setButtonStyle(m_runButton, ApplicationStyle::TexturedRounded);
    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setShortcut(Qt::CTRL | Qt::Key_R);
    m_runButton->setToolTip(tr("Run") + UtilityFunctions::shortcutSymbol(m_runButton->shortcut()));
    m_runButton->setFixedSize(39, 24);

    ApplicationStyle::setButtonStyle(m_stopButton, ApplicationStyle::TexturedRounded);
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setShortcut(Qt::CTRL | Qt::Key_T);
    m_stopButton->setToolTip(tr("Stop") + UtilityFunctions::shortcutSymbol(m_stopButton->shortcut()));
    m_stopButton->setFixedSize(39, 24);

    ApplicationStyle::setButtonStyle(m_preferencesButton, ApplicationStyle::TexturedRounded);
    m_preferencesButton->setCursor(Qt::PointingHandCursor);
    m_preferencesButton->setShortcut(Qt::CTRL | Qt::Key_Comma);
    m_preferencesButton->setToolTip(tr("Show Preferences") + UtilityFunctions::shortcutSymbol(m_preferencesButton->shortcut()));
    m_preferencesButton->setFixedSize(39, 24);

    ApplicationStyle::setButtonStyle(m_projectsButton, ApplicationStyle::TexturedRounded);
    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setShortcut(Qt::CTRL | Qt::Key_P);
    m_projectsButton->setToolTip(tr("Show Projects") + UtilityFunctions::shortcutSymbol(m_projectsButton->shortcut()));
    m_projectsButton->setFixedSize(39, 24);

    ApplicationStyle::setButtonStyle(m_segmentedBar, ApplicationStyle::TexturedRounded);
    ApplicationStyle::setHighlightingDisabledForCheckedState(m_segmentedBar, true);
    m_segmentedBar->setCursor(Qt::PointingHandCursor);
    m_segmentedBar->setIconSize({18, 18});

    QAction* leftAction = m_segmentedBar->addAction();
    leftAction->setCheckable(true);
    leftAction->setShortcut(Qt::CTRL | Qt::Key_J);
    leftAction->setToolTip(tr("Hide or show left panes") + UtilityFunctions::shortcutSymbol(leftAction->shortcut()));

    QAction* bottomAction = m_segmentedBar->addAction();
    bottomAction->setCheckable(true);
    bottomAction->setShortcut(Qt::CTRL | Qt::Key_K);
    bottomAction->setToolTip(tr("Hide or show bottom panes") + UtilityFunctions::shortcutSymbol(bottomAction->shortcut()));

    QAction* rightAction = m_segmentedBar->addAction();
    rightAction->setCheckable(true);
    rightAction->setShortcut(Qt::CTRL | Qt::Key_L);
    rightAction->setToolTip(tr("Hide or show right panes") + UtilityFunctions::shortcutSymbol(rightAction->shortcut()));

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

QPushButton* RunPane::runButton() const
{
    return m_runButton;
}

QPushButton* RunPane::stopButton() const
{
    return m_stopButton;
}

QPushButton* RunPane::projectsButton() const
{
    return m_projectsButton;
}

QPushButton* RunPane::preferencesButton() const
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
    m_runDevicesButton->setIcon(QIcon(":/images/run/devices.svg"));
    m_runButton->setIcon(renderButtonIcon(":/images/run/run.svg", m_runButton->iconSize(), this));
    m_stopButton->setIcon(renderButtonIcon(":/images/run/stop.svg", m_stopButton->iconSize(), this));
    m_preferencesButton->setIcon(renderButtonIcon(":/images/run/settings.svg", m_preferencesButton->iconSize(), this));
    m_projectsButton->setIcon(renderButtonIcon(":/images/run/projects.svg", m_projectsButton->iconSize(), this));

    QStringList iconFileNames {
        ":/images/run/left.svg",
        ":/images/run/bottom.svg",
        ":/images/run/right.svg"
    };
    for (int i = 0; i < iconFileNames.size(); ++i) {
        QAction* action = m_segmentedBar->actions().at(i);
        action->setIcon(renderButtonIcon(iconFileNames.at(i), m_segmentedBar->iconSize(), this));
    }
}

void RunPane::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ApplicationFontChange
            || event->type() == QEvent::PaletteChange) {
        updateIcons();
    }
    QToolBar::changeEvent(event);
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
}
