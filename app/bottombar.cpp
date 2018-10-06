#include <bottombar.h>

BottomBar::BottomBar(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

}

QSize BottomBar::sizeHint() const
{
    return QSize(100, 25);
}

QSize BottomBar::minimumSizeHint() const
{
    return QSize(0, 25);
}
