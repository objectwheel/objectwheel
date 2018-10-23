#include <focuslesslineedit.h>
#include <QFocusEvent>

/* FocuslessLineEdit: This widget should never have initial focus
 * (ie, be the first widget of a dialog, else, the hint cannot be displayed.
 * As it is the only focusable control in the widget box, it clears the focus
 * policy and focusses explicitly on click only (note that setting Qt::ClickFocus
 * is not sufficient for that as an ActivationFocus will occur). */

FocuslessLineEdit::FocuslessLineEdit(QWidget* parent) : QLineEdit(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

void FocuslessLineEdit::mousePressEvent(QMouseEvent* event)
{
    if (!hasFocus()) // Explicitly focus on click.
        setFocus(Qt::OtherFocusReason);
    QLineEdit::mousePressEvent(event);
}

void FocuslessLineEdit::focusInEvent(QFocusEvent* event)
{
    // Refuse the focus if the mouse it outside. In addition to the mouse
    // press logic, this prevents a re-focussing which occurs once
    // we actually had focus
    const Qt::FocusReason reason = event->reason();
    if (reason == Qt::ActiveWindowFocusReason || reason == Qt::PopupFocusReason) {
        const QPoint mousePos = mapFromGlobal(QCursor::pos());
        const bool refuse = !geometry().contains(mousePos);
        if (refuse) {
            event->ignore();
            return;
        }
    }
    QLineEdit::focusInEvent(event);
}
