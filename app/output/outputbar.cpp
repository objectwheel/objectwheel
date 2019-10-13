#include <outputbar.h>
#include <buttonflasher.h>
#include <pushbutton.h>

#include <QPainter>
#include <QBoxLayout>

static const char flasherProperty[] = "_q_OutputBar_flasher";

OutputBar::OutputBar(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(3, 2, 3, 2);
    layout->addStretch();
}

QList<QAbstractButton*> OutputBar::buttons() const
{
    QList<QAbstractButton*> buttons;
    for (int i = 0; i < layout()->count() - 1; ++i)
        buttons.append(static_cast<QAbstractButton*>(layout()->itemAt(i)->widget()));
    return buttons;
}

QAbstractButton* OutputBar::addButton()
{
    auto button = new PushButton(this);
    button->setProperty(flasherProperty, qintptr(new ButtonFlasher(button)));
    static_cast<QHBoxLayout*>(layout())->insertWidget(layout()->count() - 1, button, 0, Qt::AlignVCenter);
    return button;
}

void OutputBar::flash(QAbstractButton* button)
{
    if (auto flasher
            = reinterpret_cast<ButtonFlasher*>(button->property(flasherProperty).value<qintptr>())) {
        flasher->flash();
    }
}

void OutputBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().window());
    painter.setPen("#b6b6b6");
    painter.drawLine(rect().topLeft() + QPointF(0.5, 0.0),
                     rect().topRight() + QPointF(0.5, 0.0));
    painter.drawLine(rect().topLeft() + QPointF(0.0, 0.5),
                     rect().bottomLeft() + QPointF(0.0, 0.5));
    painter.drawLine(rect().topRight() + QPointF(1.0, 0.5),
                     rect().bottomRight() + QPointF(1.0, 0.5));
}
