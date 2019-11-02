#include <buttongroup.h>
#include <QAbstractButton>

ButtonGroup::ButtonGroup(QObject* parent) : QObject(parent)
{
}

QAbstractButton* ButtonGroup::checkedButton() const
{
    for (QAbstractButton* button : qAsConst(m_buttons)) {
        if (button->isChecked())
            return button;
    }
    return nullptr;
}

void ButtonGroup::addButton(QAbstractButton* button)
{
    m_buttons.append(button);
    connect(button, &QAbstractButton::clicked, this, [=] (bool checked) {
        emit buttonClicked(button, checked);
    });
    connect(button, &QAbstractButton::toggled, this, [=] (bool checked) {
        if (checked) {
            for (QAbstractButton* btn : qAsConst(m_buttons)) {
                if (btn != button)
                    btn->setChecked(false);
            }
        }
        emit buttonToggled(button, checked);
    });
}

void ButtonGroup::uncheckAll()
{
    for (QAbstractButton* button : qAsConst(m_buttons))
        button->setChecked(false);
}
