#include <projectoptionswidget.h>
#include <fit.h>
#include <themechooserwidget.h>

#include <QScrollArea>
#include <QGridLayout>

ProjectOptionsWidget::ProjectOptionsWidget(QWidget *parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_scrollArea(new QScrollArea)
  , m_containerWidget(new QWidget)
  , m_gridLayout(new QGridLayout(m_containerWidget))
  , m_themeChooserWidget(new ThemeChooserWidget(ThemeChooserWidget::V1))
  , m_themeChooserWidget_2(new ThemeChooserWidget(ThemeChooserWidget::V2))
{
    m_layout->addWidget(m_scrollArea);

    m_gridLayout->addWidget(m_themeChooserWidget, 0, 0);
    m_gridLayout->addWidget(m_themeChooserWidget_2, 1, 0);

    m_scrollArea->setWidget(m_containerWidget);
    m_scrollArea->setWidgetResizable(true);
}

QSize ProjectOptionsWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
}

void ProjectOptionsWidget::reset()
{
    //TODO
}
