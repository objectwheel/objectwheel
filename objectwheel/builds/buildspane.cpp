#include <buildspane.h>
#include <stackedlayout.h>
#include <platformselectionwidget.h>
#include <androidplatformwidget.h>
#include <downloadwidget.h>

#include <QScrollArea>
#include <QGridLayout>

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

QSize BuildsPane::sizeHint() const
{
    return {670, 640};
}

QSize BuildsPane::minimumSizeHint() const
{
    return {0, 0};
}
