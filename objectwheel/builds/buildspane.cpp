#include <buildspane.h>
#include <buttonslice.h>
#include <stackedlayout.h>
#include <platformselectionwidget.h>
#include <androidplatformwidget.h>
#include <applicationstyle.h>

#include <QScrollArea>
#include <QGridLayout>
#include <QAction>
#include <QLabel>
#include <QPushButton>

BuildsPane::BuildsPane(QWidget* parent) : QScrollArea(parent)
  , m_platformLabel(new QLabel(this))
  , m_buttonSlice(new ButtonSlice(this))
  , m_stackedLayout(new StackedLayout)
  , m_platformSelectionWidget(new PlatformSelectionWidget(this))
  , m_androidPlatformWidget(new AndroidPlatformWidget(this))
{
    setWidgetResizable(true);
    setWidget(new QWidget(this));
    setFrameShape(QFrame::NoFrame);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_buttonSlice);
    buttonLayout->addStretch();

    auto layout = new QGridLayout(widget());
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->addWidget(m_platformLabel, 0, 1, Qt::AlignHCenter);
    layout->addLayout(m_stackedLayout, 1, 1);
    layout->addLayout(buttonLayout, 2, 1);
    layout->setRowStretch(3, 1);

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_platformLabel->setFont(f);

    m_stackedLayout->addWidget(m_platformSelectionWidget);
    m_stackedLayout->addWidget(m_androidPlatformWidget);
    m_stackedLayout->setCurrentWidget(m_platformSelectionWidget);

    m_buttonSlice->add(Back, "#5BC5F8", "#2592F9");
    m_buttonSlice->add(Next, "#8BBB56", "#6EA045");
    m_buttonSlice->get(Back)->setText(tr("Back"));
    m_buttonSlice->get(Next)->setText(tr("Next"));
    m_buttonSlice->get(Back)->setIcon(QIcon(":/images/welcome/unload.png"));
    m_buttonSlice->get(Next)->setIcon(QIcon(":/images/welcome/load.png"));
    m_buttonSlice->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttonSlice->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttonSlice->settings().cellWidth = 150;
    m_buttonSlice->triggerSettings();
}

QLabel* BuildsPane::platformLabel() const
{
    return m_platformLabel;
}

ButtonSlice* BuildsPane::buttonSlice() const
{
    return m_buttonSlice;
}

StackedLayout* BuildsPane::stackedLayout() const
{
    return m_stackedLayout;
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
