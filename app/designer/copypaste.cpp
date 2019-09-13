#include <copypaste.h>

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

QList<QPointer<Control>> CopyPaste::controls()
{
    return m_controls;
}

void CopyPaste::setControls(const QList<QPointer<Control>>& controls)
{
    m_controls = controls;
}
