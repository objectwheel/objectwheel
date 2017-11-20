#include <filterlineedit.h>
#include <QFocusEvent>

/* FilterLineEdit: This widget should never have initial focus
 * (ie, be the first widget of a dialog, else, the hint cannot be displayed.
 * As it is the only focusable control in the widget box, it clears the focus
 * policy and focusses explicitly on click only (note that setting Qt::ClickFocus
 * is not sufficient for that as an ActivationFocus will occur). */

FilterLineEdit::FilterLineEdit(QWidget* parent)
    : QLineEdit(parent)
    , m_defaultFocusPolicy(focusPolicy())
{
    setFocusPolicy(Qt::NoFocus);
}

void FilterLineEdit::mousePressEvent(QMouseEvent *e)
{
    if (!hasFocus()) // Explicitly focus on click.
        setFocus(Qt::OtherFocusReason);
    QLineEdit::mousePressEvent(e);
}

void FilterLineEdit::focusInEvent(QFocusEvent *e)
{
    // Refuse the focus if the mouse it outside. In addition to the mouse
    // press logic, this prevents a re-focussing which occurs once
    // we actually had focus
    const Qt::FocusReason reason = e->reason();
    if (reason == Qt::ActiveWindowFocusReason || reason == Qt::PopupFocusReason) {
        const QPoint mousePos = mapFromGlobal(QCursor::pos());
        const bool refuse = !geometry().contains(mousePos);
        if (refuse) {
            e->ignore();
            return;
        }
    }
    QLineEdit::focusInEvent(e);
}
