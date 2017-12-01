#include <view.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#define SWIPE_DURATION (300)

void swipe(QWidget* w1, QWidget* w2, QLayout* layout, View::Direction direction)
{
    static auto a1 = new QPropertyAnimation;
    static auto a2 = new QPropertyAnimation;
    static auto g = new QParallelAnimationGroup;

    a1->setTargetObject(w1);
    a2->setTargetObject(w2);
    a1->setStartValue(w1->geometry());
    a2->setEndValue(w1->geometry());

    if (g->animationCount() == 0) {
        a1->setEasingCurve(QEasingCurve::OutExpo);
        a2->setEasingCurve(QEasingCurve::OutExpo);
        a1->setDuration(SWIPE_DURATION);
        a2->setDuration(SWIPE_DURATION);
        a1->setPropertyName("geometry");
        a2->setPropertyName("geometry");
        g->addAnimation(a1);
        g->addAnimation(a2);
    }

    switch (direction) {
        case View::LeftToRight:
            a1->setEndValue(QRect(QPoint(w1->x() + w1->width(), w1->y()), w1->size()));
            a2->setStartValue(QRect(QPoint(w1->x() - w1->width(), w1->y()), w1->size()));
            g->start();
            break;

        case View::RightToLeft:
            a1->setEndValue(QRect(QPoint(w1->x() - w1->width(), w1->y()), w1->size()));
            a2->setStartValue(QRect(QPoint(w1->x() + w1->width(), w1->y()), w1->size()));
            g->start();
            break;

        default:
            break;
    }

    QObject::connect(g, &QParallelAnimationGroup::finished, [=]{
        w1->hide();
        layout->addWidget(w2);
        g->disconnect();
    });
}

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

void View::show(int id, Direction direction)
{
    QWidget* w;
    if ((w = _widgets.value(id)) == 0)
        return;

    QLayoutItem* li = _layout.takeAt(0);
    if (direction == NoSwipe) {
        if (li && li->widget())
            li->widget()->hide();

        _layout.addWidget(w);
        w->show();
    } else {
        if (li && li->widget()) {
            w->setParent(this);
            w->show();
            swipe(li->widget(), w, &_layout, direction);
        } else {
            _layout.addWidget(w);
            w->show();
        }
    }

    emit visibleChanged();
}

int View::id(QWidget* widget) const
{
    return _widgets.key(widget, -1);
}
