#ifndef FORMSCENE_H
#define FORMSCENE_H

#include <controlscene.h>

class FormScene : public ControlScene
{
        Q_OBJECT

    public:
        explicit FormScene(QObject *parent = Q_NULLPTR);
        void setMainControl(Control* mainControl) override;

        const QList<Form*>& forms() const;
        void addForm(Form* form);
        void removeForm(Form* form);
        void setMainForm(Form* mainForm);
        Form* mainForm();

    signals:
        void mainControlChanged(Control* mainControl);

    private:
        QList<Form*> _forms;
};

#endif // FORMSCENE_H
