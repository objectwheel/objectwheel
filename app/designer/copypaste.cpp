#include <copypaste.h>
#include <control.h>

CopyPaste::CopyPaste() : m_actionType(Invalid)
{
}

bool CopyPaste::isValid()
{
    return m_actionType != Invalid && !m_controls.isEmpty();
}

void CopyPaste::invalidate()
{
    m_actionType = Invalid;
    m_controls.clear();
}

CopyPaste::ActionType CopyPaste::actionType()
{
    return m_actionType;
}

void CopyPaste::setActionType(const ActionType& actionType)
{
    m_actionType = actionType;
}

QList<Control*> CopyPaste::controls()
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
