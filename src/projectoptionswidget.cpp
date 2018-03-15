#include <projectoptionswidget.h>
#include <fit.h>

ProjectOptionsWidget::ProjectOptionsWidget(QWidget *parent) : QWidget(parent)
{

}

QSize ProjectOptionsWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
}

void ProjectOptionsWidget::reset()
{
    //TODO
}
