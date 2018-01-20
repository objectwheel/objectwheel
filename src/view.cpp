#include <view.h>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#define SWIPE_DURATION 500

void swipe(QWidget* w1, QWidget* w2, QLayout* layout, View::SwipeDirection direction)
{
    static auto a1 = new QPropertyAnimation;
    static auto a2 = new QPropertyAnimation;
    static auto ag = new QParallelAnimationGroup;

    if (ag->animationCount() == 0) {
        a1->setEasingCurve(QEasingCurve::OutQuart);
        a2->setEasingCurve(QEasingCurve::OutQuart);
        a1->setDuration(SWIPE_DURATION);
        a2->setDuration(SWIPE_DURATION);
        a1->setPropertyName("geometry");
        a2->setPropertyName("geometry");
        ag->addAnimation(a1);
        ag->addAnimation(a2);
    }

    a1->setTargetObject(w1);
    a2->setTargetObject(w2);
    a1->setStartValue(w1->geometry());
    a2->setEndValue(w1->geometry());

    switch (direction) {
        case View::LeftToRight:
            a1->setEndValue(QRect(QPoint(w1->x() + w1->width(), w1->y()), w1->size()));
            a2->setStartValue(QRect(QPoint(w1->x() - w1->width(), w1->y()), w1->size()));
            break;

        case View::RightToLeft:
            a1->setEndValue(QRect(QPoint(w1->x() - w1->width(), w1->y()), w1->size()));
            a2->setStartValue(QRect(QPoint(w1->x() + w1->width(), w1->y()), w1->size()));
            break;

        default:
            Q_ASSERT(0);
            break;
    }

    QObject::connect(ag, &QParallelAnimationGroup::finished, [=] {
        w1->hide();
        layout->update();
        ag->disconnect();
    });

    ag->start();
}

View::View(QWidget* parent) : QWidget(parent)
  , _visibleId(-1)
{
    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);
}

void View::add(int id, QWidget* widget)
{
    Q_ASSERT(id >= 0);
    _widgets[id] = widget;
    _layout->addWidget(widget);
    widget->hide();
}

void View::show(int id, SwipeDirection direction)
{
    QWidget* w1 = _widgets.value(_visibleId),* w2;
    if ((w2 = _widgets.value(id)) == 0)
        return;

    w2->show();

    if (w1) {
        if (direction == NoSwipe)
            w1->hide();
        else {
            w1->setGeometry(rect());
            swipe(w1, w2, _layout, direction);
        }
    }

    _visibleId = id;
}