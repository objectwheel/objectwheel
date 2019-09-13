#include <copypaste.h>
#include <control.h>

CopyPaste::CopyPaste()
    : m_copyCount(0)
    , m_actionType(Invalid)
{
}

bool CopyPaste::isValid() const
{
    return m_actionType != Invalid && !m_controls.isEmpty();
}

void CopyPaste::invalidate()
{
    *this = CopyPaste();
}

int CopyPaste::copyCount() const
{
    return m_copyCount;
}

void CopyPaste::increaseCopyCount()
{
    m_copyCount++;
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
