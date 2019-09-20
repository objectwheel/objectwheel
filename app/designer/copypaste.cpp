#include <copypaste.h>
#include <control.h>

CopyPaste::CopyPaste()
    : m_count(0)
    , m_counterControl(nullptr)
    , m_actionType(Invalid)
{
}

bool CopyPaste::isValid() const
{
    bool valid = m_actionType != Invalid && !m_controls.isEmpty();
    if (!valid)
        return false;
    for (Control* control : m_controls) {
        if (control)
            return true;
    }
    return false;
}

void CopyPaste::invalidate()
{
    *this = CopyPaste();
}

int CopyPaste::count() const
{
    return m_count;
}

void CopyPaste::increaseCount(Control* counterControl)
{
    if (m_counterControl != counterControl) {
        m_counterControl = counterControl;
        m_count = 1;
    } else {
        m_count++;
    }
}

QPointF CopyPaste::pos() const
{
    return m_pos;
}

void CopyPaste::setPos(const QPointF& pos)
{
    m_pos = pos;
}

CopyPaste::ActionType CopyPaste::actionType() const
{
    return m_actionType;
}

void CopyPaste::setActionType(const ActionType& actionType)
{
    m_actionType = actionType;
}

QList<Control*> CopyPaste::controls() const
{
    QList<Control*> controls;
    for (Control* control : qAsConst(m_controls)) {
        if (control)
            controls.append(control);
    }
    return controls;
}

void CopyPaste::setControls(const QList<Control*>& controls)
{
    m_controls.clear();
    for (Control* control : controls)
        m_controls.append(control);
}
