#include <projectoptionswidget.h>
#include <themechooserwidget.h>
#include <scalingwidget.h>

#include <QScrollArea>
#include <QGridLayout>

ProjectOptionsWidget::ProjectOptionsWidget(QWidget *parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_scrollArea(new QScrollArea)
  , m_containerWidget(new QWidget)
  , m_gridLayout(new QGridLayout(m_containerWidget))
  , m_themeChooserWidget(new ThemeChooserWidget(ThemeChooserWidget::V1))
  , m_themeChooserWidget_2(new ThemeChooserWidget(ThemeChooserWidget::V2))
  , m_scalingWidget(new ScalingWidget)
{
    m_layout->addWidget(m_scrollArea);

    m_gridLayout->addWidget(m_themeChooserWidget, 0, 0);
    m_gridLayout->addWidget(m_themeChooserWidget_2, 1, 0);
    m_gridLayout->addWidget(m_scalingWidget, 2, 0);

    m_scrollArea->setWidget(m_containerWidget);
    m_scrollArea->setWidgetResizable(true);
}

QSize ProjectOptionsWidget::sizeHint() const
{
    return QSize(680, 680);
}

void ProjectOptionsWidget::reset()
{
    m_themeChooserWidget->reset();
    m_themeChooserWidget_2->reset();
    m_scalingWidget->reset();
}
