#ifndef COPYPASTE_H
#define COPYPASTE_H

#include <QPointer>

class Control;

struct CopyPaste final
{
    Q_DISABLE_COPY(CopyPaste)

    enum ActionType { Invalid, Copy, Cut };

    CopyPaste();

    bool isValid();
    void invalidate();

    ActionType actionType();
    void setActionType(const ActionType& actionType);

    QList<QPointer<Control>> controls();
    void setControls(const QList<QPointer<Control>>& controls);

private:
    ActionType m_actionType;
    QList<QPointer<Control>> m_controls;
};

#endif // COPYPASTE_H
