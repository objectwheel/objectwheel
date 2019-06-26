#ifndef BUTTONFLASHER_H
#define BUTTONFLASHER_H

#include <QObject>
#include <QBasicTimer>

class QAbstractButton;
class ButtonFlasher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ButtonFlasher)

public:
    explicit ButtonFlasher(QAbstractButton* parent);

    void flash(int timeout, int repeat);

private:
    void timerEvent(QTimerEvent* event) override;

private:
    int m_repeat;
    QBasicTimer m_flashTimer;
    QAbstractButton* m_button;
};

#endif // BUTTONFLASHER_H
