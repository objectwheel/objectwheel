#ifndef COPYPASTE_H
#define COPYPASTE_H

#include <QPointer>

class Control;

struct CopyPaste final
{
    enum ActionType { Invalid, Copy, Cut };

    CopyPaste();

    bool isValid() const;
    void invalidate();

    int copyCount() const;
    void increaseCopyCount();

    ActionType actionType() const;
    void setActionType(const ActionType& actionType);

    QList<Control*> controls() const;
    void setControls(const QList<Control*>& controls);

private:
    int m_copyCount;
    ActionType m_actionType;
    QList<QPointer<Control>> m_controls;
};

#endif // COPYPASTE_H
