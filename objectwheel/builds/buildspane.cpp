#include <buildspane.h>
#include <stackedlayout.h>
#include <platformselectionwidget.h>
#include <androidplatformwidget.h>
#include <downloadwidget.h>
#include <tooltip/tooltip.h>

#include <QScrollArea>
#include <QGridLayout>
#include <QHelpEvent>
#include <QCoreApplication>

BuildsPane::BuildsPane(QWidget* parent) : QScrollArea(parent)
  , m_stackedLayout(new StackedLayout)
  , m_downloadWidget(new DownloadWidget(this))
  , m_platformSelectionWidget(new PlatformSelectionWidget(this))
  , m_androidPlatformWidget(new AndroidPlatformWidget(this))
{
    setWidgetResizable(true);
    setWidget(new QWidget(this));
    setFrameShape(QFrame::NoFrame);

    auto layout = new QGridLayout(widget());
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(0, 1);
    layout->addLayout(m_stackedLayout, 1, 1);
    layout->setRowStretch(2, 1);

    m_stackedLayout->addWidget(m_downloadWidget);
    m_stackedLayout->addWidget(m_platformSelectionWidget);
    m_stackedLayout->addWidget(m_androidPlatformWidget);

    // To enable clickable links in tool
    // tips we use our own ToolTip class
    QCoreApplication::instance()->installEventFilter(this);
}

StackedLayout* BuildsPane::stackedLayout() const
{
    return m_stackedLayout;
}

DownloadWidget* BuildsPane::downloadWidget() const
{
    return m_downloadWidget;
}

PlatformSelectionWidget* BuildsPane::platformSelectionWidget() const
{
    return m_platformSelectionWidget;
}

AndroidPlatformWidget* BuildsPane::androidPlatformWidget() const
{
    return m_androidPlatformWidget;
}

bool BuildsPane::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {
        if (auto widget = qobject_cast<QWidget*>(watched)) {
            if (isAncestorOf(widget)) {
                if (widget->toolTip().isEmpty()) {
                    event->ignore();
                } else {
                    Utils::ToolTip::show(static_cast<QHelpEvent*>(event)->globalPos(),
                                         widget->toolTip(), widget);
                }
                return true;
            }
        }
    }
    return QScrollArea::eventFilter(watched, event);
}

QSize BuildsPane::sizeHint() const
{
    return {670, 640};
}

QSize BuildsPane::minimumSizeHint() const
{
    return {0, 0};
}
