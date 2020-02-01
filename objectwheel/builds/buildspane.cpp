#include <buildspane.h>
#include <androidplatformwidget.h>

#include <QStackedWidget>
#include <QScrollArea>
#include <QGridLayout>

BuildsPane::BuildsPane(QWidget* parent) : QWidget(parent)
  , m_stackedWidget(new QStackedWidget(this))
  , m_androidWidget(new AndroidPlatformWidget(this))
{    
    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_stackedWidget);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->addWidget(scrollArea, 1, 1);

    m_stackedWidget->addWidget(m_androidWidget);
    m_stackedWidget->setCurrentWidget(m_androidWidget);
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
