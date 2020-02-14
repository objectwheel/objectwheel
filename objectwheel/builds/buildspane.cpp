#include <buildspane.h>
#include <buttonslice.h>
#include <stackedlayout.h>
#include <platformselectionwidget.h>
#include <androidplatformwidget.h>
#include <applicationstyle.h>
#include <downloadwidget.h>

#include <QScrollArea>
#include <QGridLayout>
#include <QAction>
#include <QLabel>
#include <QPushButton>

BuildsPane::BuildsPane(QWidget* parent) : QScrollArea(parent)
  , m_iconLabel(new QLabel(this))
  , m_platformLabel(new QLabel(this))
  , m_descriptionLabel(new QLabel(this))
  , m_buttonSlice(new ButtonSlice(this))
  , m_stackedLayout(new StackedLayout)
  , m_downloadWidget(new DownloadWidget(this))
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
    layout->setRowStretch(0, 1);
    layout->addWidget(m_iconLabel, 1, 1, Qt::AlignHCenter);
    layout->addWidget(m_platformLabel, 2, 1, Qt::AlignHCenter);
    layout->addWidget(m_descriptionLabel, 3, 1, Qt::AlignHCenter);
    layout->addLayout(m_stackedLayout, 4, 1);
    layout->addLayout(buttonLayout, 5, 1);
    layout->setRowStretch(6, 1);

    m_iconLabel->setFixedSize(QSize(60, 60));
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_platformLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setAlignment(Qt::AlignCenter);

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_platformLabel->setFont(f);

    m_descriptionLabel->setStyleSheet(
                QLatin1String("QLabel {"
                              "  padding: 9px;"
                              "  color: white;"
                              "  background: #62000000;"
                              "  border: none;"
                              "  border-radius: 3px"
                              "}"));

    m_stackedLayout->addWidget(m_downloadWidget);
    m_stackedLayout->addWidget(m_platformSelectionWidget);
    m_stackedLayout->addWidget(m_androidPlatformWidget);

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

QLabel* BuildsPane::iconLabel() const
{
    return m_iconLabel;
}

QLabel* BuildsPane::descriptionLabel() const
{
    return m_descriptionLabel;
}
