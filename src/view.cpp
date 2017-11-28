#include <view.h>

View::View(QWidget* parent) : QWidget(parent)
{
    _layout.setContentsMargins(0, 0, 0, 0);
    _layout.setSpacing(0);
    setLayout(&_layout);
}

void View::add(int id, QWidget* widget)
{
    _widgets[id] = widget;
}

int View::remove(int id)
{
    QLayoutItem* li = _layout.itemAt(0);
    if (li && li->widget() && li->widget() == _widgets.value(id))
        layout()->takeAt(0);
    return _widgets.remove(id);
}

QWidget* View::widget(int id) const
{
    return _widgets.value(id);
}

int View::current() const
{
    QLayoutItem* li = _layout.itemAt(0);
    if (li && li->widget())
        return id(li->widget());
    return -1;
}

void View::show(int id)
{
    QWidget* w;
    if ((w = _widgets.value(id)) == 0)
        return;

    QLayoutItem* li = _layout.takeAt(0);
    if (li && li->widget())
        li->widget()->hide();

    _layout.addWidget(w);
    w->show();

    emit visibleChanged();
}

int View::id(QWidget* widget) const
{
    return _widgets.key(widget, -1);
}
