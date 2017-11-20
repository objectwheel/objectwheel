#include <centralwidget.h>

CentralWidget::CentralWidget(QMainWindow* parent)
    : QWidget(parent)
{
    parent->setCentralWidget(this);

    _layout.setContentsMargins(0, 0, 0, 0);
    _layout.setSpacing(0);
    setLayout(&_layout);
}

void CentralWidget::addWidget(int uid, QWidget* widget)
{
    _widgetMap[uid] = widget;
}

int CentralWidget::removeWidget(int uid)
{
    QLayoutItem* li = _layout.itemAt(0);
    if (li && li->widget() && li->widget() == _widgetMap.value(uid))
        layout()->takeAt(0);
    return _widgetMap.remove(uid);
}

QWidget* CentralWidget::widget(int uid) const
{
    return _widgetMap.value(uid);
}

int CentralWidget::visibleUid() const
{
    QLayoutItem* li = _layout.itemAt(0);
    if (li && li->widget())
        return uid(li->widget());
    return -1;
}

void CentralWidget::showWidget(int uid)
{
    QWidget* w;
    if ((w = _widgetMap.value(uid)) == 0)
        return;

    QLayoutItem* li = _layout.takeAt(0);
    if (li && li->widget())
        li->widget()->hide();

    _layout.addWidget(w);
    w->show();

    emit visibleWidgetChanged();
}

int CentralWidget::uid(QWidget* widget) const
{
    return _widgetMap.key(widget, -1);
}
