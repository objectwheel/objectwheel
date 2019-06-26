#include <buttonflasher.h>
#include <QAbstractButton>
#include <QTimerEvent>

ButtonFlasher::ButtonFlasher(QAbstractButton* parent) : QObject(parent)
  , m_button(parent)
{
}

void ButtonFlasher::flash(int timeout, int repeat)
{
    m_repeat = 2 * repeat - 1;
    m_button->setDown(true);
    m_flashTimer.stop();
    m_flashTimer.start(timeout, this);
}

void ButtonFlasher::timerEvent(QTimerEvent* event)
{
    if (m_flashTimer.timerId() == event->timerId()) {
        if (--m_repeat < 0) {
            m_flashTimer.stop();
            return;
        }
        if (m_button->isDown())
            m_button->setDown(false);
        else
            m_button->setDown(true);
    } else {
        QObject::timerEvent(event);
    }
}
