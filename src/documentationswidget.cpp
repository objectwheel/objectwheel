#include <documentationswidget.h>
#include <fit.h>

DocumentationsWidget::DocumentationsWidget(QWidget *parent) : QWidget(parent)
{

}

QSize DocumentationsWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
}

void DocumentationsWidget::reset()
{
    //TODO
}
