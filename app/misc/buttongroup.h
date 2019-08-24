#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include <QObject>

class QAbstractButton;
class ButtonGroup final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ButtonGroup)

public:
    explicit ButtonGroup(QObject* parent = nullptr);

    void addButton(QAbstractButton* button);

    void uncheckAll();

private:
    QList<QAbstractButton*> m_buttons;
};

#endif // BUTTONGROUP_H
