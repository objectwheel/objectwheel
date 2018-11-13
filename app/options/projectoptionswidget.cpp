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
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_gridLayout->addWidget(m_themeChooserWidget, 0, 0);
    m_gridLayout->addWidget(m_themeChooserWidget_2, 1, 0);
    m_gridLayout->addWidget(m_scalingWidget, 2, 0);

    m_scrollArea->setWidget(m_containerWidget);
    m_scrollArea->setWidgetResizable(true);

    connect(m_themeChooserWidget, &ThemeChooserWidget::saved, this, &ProjectOptionsWidget::themeChanged);
    connect(m_themeChooserWidget_2, &ThemeChooserWidget::saved, this, &ProjectOptionsWidget::themeChanged);
}

QSize ProjectOptionsWidget::sizeHint() const
{
    return QSize(680, 680);
}

void ProjectOptionsWidget::discharge()
{
    m_themeChooserWidget->discharge();
    m_themeChooserWidget_2->discharge();
    m_scalingWidget->discharge();
}
