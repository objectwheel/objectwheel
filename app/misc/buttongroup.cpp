#include <buttongroup.h>
#include <QAbstractButton>

ButtonGroup::ButtonGroup(QObject* parent) : QObject(parent)
{
}

void ButtonGroup::addButton(QAbstractButton* button)
{
    m_buttons.append(button);
    connect(button, &QAbstractButton::toggled, this, [=] (bool checked) {
        if (checked) {
            for (QAbstractButton* btn : qAsConst(m_buttons)) {
                if (btn != button)
                    btn->setChecked(false);
            }
        }
    });
}

void ButtonGroup::uncheckAll()
{
    for (QAbstractButton* button : qAsConst(m_buttons))
        button->setChecked(false);
}
