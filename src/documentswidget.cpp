#include <documentswidget.h>
#include <fit.h>

DocumentsWidget::DocumentsWidget(QWidget *parent) : QWidget(parent)
{

}

QSize DocumentsWidget::sizeHint() const
{
    return fit::fx(QSizeF(680, 680)).toSize();
}

void DocumentsWidget::reset()
{
    //TODO
}
