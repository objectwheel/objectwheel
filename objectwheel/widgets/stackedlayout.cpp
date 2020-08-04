#include <stackedlayout.h>
#include <QWidget>

StackedLayout::StackedLayout() : QStackedLayout()
{
}

StackedLayout::StackedLayout(QWidget* parent) : QStackedLayout(parent)
{
}

StackedLayout::StackedLayout(QLayout* parentLayout) : QStackedLayout(parentLayout)
{
}

QSize StackedLayout::sizeHint() const
{
    if (currentWidget())
        return currentWidget()->sizeHint();
    return QStackedLayout::sizeHint();
}

QSize StackedLayout::minimumSize() const
{
    if (currentWidget())
        return currentWidget()->minimumSizeHint();
    return QStackedLayout::minimumSize();
}
