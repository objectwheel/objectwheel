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
  , m_themeChooserWidget1(new ThemeChooserWidget(ThemeChooserWidget::V1))
  , m_themeChooserWidget(new ThemeChooserWidget(ThemeChooserWidget::V2))
  , m_scalingWidget(new ScalingWidget)
{
    m_layout->addWidget(m_scrollArea);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_gridLayout->addWidget(m_themeChooserWidget, 0, 0);
    m_gridLayout->addWidget(m_themeChooserWidget1, 1, 0);
    m_gridLayout->addWidget(m_scalingWidget, 2, 0);

    m_scrollArea->setWidget(m_containerWidget);
    m_scrollArea->setWidgetResizable(true);

    connect(m_themeChooserWidget1, &ThemeChooserWidget::saved, this, &ProjectOptionsWidget::themeChanged);
    connect(m_themeChooserWidget, &ThemeChooserWidget::saved, this, &ProjectOptionsWidget::themeChanged);
}

ThemeChooserWidget* ProjectOptionsWidget::themeChooserWidget() const
{
    return m_themeChooserWidget;
}

ThemeChooserWidget* ProjectOptionsWidget::themeChooserWidget1() const
{
    return m_themeChooserWidget1;
}

QSize ProjectOptionsWidget::sizeHint() const
{
    return QSize(640, 640);
}

void ProjectOptionsWidget::charge()
{
    m_themeChooserWidget1->charge();
    m_themeChooserWidget->charge();
    m_scalingWidget->charge();
}
