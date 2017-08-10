#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <controlscene.h>

class FormScene : public ControlScene
{
        Q_OBJECT

    public:
        explicit FormScene(QObject *parent = Q_NULLPTR);
        const QList<Form*>& forms() const { return _forms; }
        void addForm(Form* form);
        void removeForm(Form* form);
        Form* mainForm();
        void setMainForm(Form* mainForm);
        void setMainControl(Control* mainControl) override;

    private:
        QList<Form*> _forms;
};

#endif // FORMSCENE_H
