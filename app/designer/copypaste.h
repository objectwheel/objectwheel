#ifndef COPYPASTE_H
#define COPYPASTE_H

#include <QPointer>
#include <QPointF>

class Control;

struct CopyPaste final
{
    enum ActionType { Invalid, Copy, Cut };

    CopyPaste();

    bool isValid() const;
    void invalidate();

    int count() const;
    void increaseCount();

    QPointF pos() const;
    void setPos(const QPointF& pos);

    ActionType actionType() const;
    void setActionType(const ActionType& actionType);

    QList<Control*> controls() const;
    void setControls(const QList<Control*>& controls);

private:
    int m_count;
    QPointF m_pos;
    ActionType m_actionType;
    QList<QPointer<Control>> m_controls;
};

#endif // COPYPASTE_H
