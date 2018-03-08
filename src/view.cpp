#include <view.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#define SWIPE_DURATION 500

View::View(QWidget* parent) : QWidget(parent)
  , m_visibleId(-1)
  , m_animationGroup(new QParallelAnimationGroup(this))
  , m_animationBack(new QPropertyAnimation(this))
  , m_animationForth(new QPropertyAnimation(this))
{
    m_animationBack->setDuration(SWIPE_DURATION);
    m_animationForth->setDuration(SWIPE_DURATION);
    m_animationBack->setPropertyName("geometry");
    m_animationForth->setPropertyName("geometry");
    m_animationBack->setEasingCurve(QEasingCurve::OutQuart);
    m_animationForth->setEasingCurve(QEasingCurve::OutQuart);
    m_animationGroup->addAnimation(m_animationBack);
    m_animationGroup->addAnimation(m_animationForth);
}

void View::add(int id, QWidget* widget)
{
    Q_ASSERT(id >= 0);

    m_widgets[id] = widget;
    widget->setParent(this);
    widget->hide();
}

void View::show(int id, SwipeDirection direction)
{
    QWidget* w1 = m_widgets.value(m_visibleId),* w2;

    if ((w2 = m_widgets.value(id)) == 0)
        return;

    w2->show();

    if (w1) {
        if (direction == NoSwipe)
            w1->hide();
        else {
            w1->setGeometry(rect());
            swipe(w1, w2, direction);
        }
    }

    m_visibleId = id;
}

void View::resizeEvent(QResizeEvent* event)
{
    if (m_widgets.contains(m_visibleId))
        m_widgets.value(m_visibleId)->setGeometry(rect());

    QWidget::resizeEvent(event);
}

void View::swipe(QWidget* w1, QWidget* w2, View::SwipeDirection direction)
{
    m_animationBack->setTargetObject(w1);
    m_animationForth->setTargetObject(w2);
    m_animationBack->setStartValue(w1->geometry());
    m_animationForth->setEndValue(w1->geometry());

    switch (direction) {
        case View::LeftToRight:
            m_animationBack->setEndValue(QRect(QPoint(w1->x() + w1->width(), w1->y()), w1->size()));
            m_animationForth->setStartValue(QRect(QPoint(w1->x() - w1->width(), w1->y()), w1->size()));
            break;

        case View::RightToLeft:
            m_animationBack->setEndValue(QRect(QPoint(w1->x() - w1->width(), w1->y()), w1->size()));
            m_animationForth->setStartValue(QRect(QPoint(w1->x() + w1->width(), w1->y()), w1->size()));
            break;

        default:
            Q_ASSERT(0);
            break;
    }

    QObject::connect(m_animationGroup, &QParallelAnimationGroup::finished, [=] {
        w1->hide();
        m_animationGroup->disconnect();
    });

    m_animationGroup->start();
}