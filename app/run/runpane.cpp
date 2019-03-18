#include <runpane.h>
#include <pushbutton.h>
#include <rundevicesbutton.h>
#include <runprogressbar.h>
#include <smartspacer.h>
#include <transparentstyle.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QPainter>
#include <QLayout>

RunPane::RunPane(QWidget* parent) : QToolBar(parent)
  , m_runButton(new PushButton)
  , m_stopButton(new PushButton)
  , m_projectsButton(new PushButton)
  , m_preferencesButton(new PushButton)
  , m_runProgressBar(new RunProgressBar)
  , m_runDevicesButton(new RunDevicesButton)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_runDevicesButton->setCursor(Qt::PointingHandCursor);
    m_runDevicesButton->setText(tr("Devices"));
    m_runDevicesButton->setIconSize({16, 16});
    m_runDevicesButton->setIcon(PaintUtils::renderButtonIcon(":/images/devices.png", this));

    m_runButton->setCursor(Qt::PointingHandCursor);
    m_runButton->setToolTip(tr("Run"));
    m_runButton->setFixedWidth(39);
    m_runButton->setIconSize({16, 16});
    m_runButton->setIcon(PaintUtils::renderMaskedButtonIcon(":/utils/images/run_small@2x.png", this));

    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setToolTip(tr("Stop"));
    m_stopButton->setFixedWidth(39);
    m_stopButton->setIconSize({16, 16});
    m_stopButton->setIcon(PaintUtils::renderMaskedButtonIcon(":/utils/images/stop_small@2x.png", this));

    m_preferencesButton->setCursor(Qt::PointingHandCursor);
    m_preferencesButton->setToolTip(tr("Show Preferences"));
    m_preferencesButton->setFixedWidth(39);
    m_preferencesButton->setIconSize({16, 16});
    m_preferencesButton->setIcon(PaintUtils::renderOverlaidButtonIcon(":/images/preferences.png", this));

    m_projectsButton->setCursor(Qt::PointingHandCursor);
    m_projectsButton->setToolTip(tr("Show Projects"));
    m_projectsButton->setFixedWidth(39);
    m_projectsButton->setIconSize({16, 16});
    m_projectsButton->setIcon(PaintUtils::renderOverlaidButtonIcon(":/images/projects.png", this));

    TransparentStyle::attach(this);
    QMetaObject::invokeMethod(this, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        setContentsMargins(0, 0, 0, 0);
        layout()->setContentsMargins(7, 7, 7, 7); // They must be all same
        layout()->setSpacing(7);
    }, Qt::QueuedConnection);

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

void RunPane::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().window());
}
