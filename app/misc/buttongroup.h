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

    QAbstractButton* checkedButton() const;

    void addButton(QAbstractButton* button);

    void uncheckAll();

signals:
    void buttonClicked(QAbstractButton* button, bool checked);
    void buttonToggled(QAbstractButton* button, bool checked);

private:
    QList<QAbstractButton*> m_buttons;
};

#endif // BUTTONGROUP_H
