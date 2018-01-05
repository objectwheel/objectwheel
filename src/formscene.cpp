#include <formscene.h>

FormScene::FormScene(QObject *parent)
    : ControlScene(parent)
{
}

void FormScene::addForm(Form* form)
{
    if (_forms.contains(form))
        return;

    addItem(form);
    form->setVisible(false);

    _forms.append(form);

    if (!_mainControl)
        setMainControl(form);
}

void FormScene::removeForm(Form* form)
{
    if (_forms.contains(form) == false ||
        form->main())
        return;

    for (auto ctrl : form->childControls())
        emit aboutToRemove(ctrl);
    emit aboutToRemove(form);

    removeItem(form);
    _forms.removeOne(form);

    if (_mainControl == form)
        setMainControl(_forms[0]);

    form->deleteLater();
    emit controlRemoved(form);
}

Form* FormScene::mainForm()
{
    return (Form*)_mainControl.data();
}

void FormScene::clearScene()
{
    _forms.clear();
    ControlScene::clearScene();
}

void FormScene::setMainForm(Form* mainForm)
{
    if (_forms.contains(mainForm) == false ||
        _mainControl == mainForm)
        return;

    if (_mainControl)
        _mainControl->setVisible(false);

    _mainControl = mainForm;
    _mainControl->setVisible(true);
}

void FormScene::setMainControl(Control* mainControl)
{
    Form* form;
    if ((form = dynamic_cast<Form*>(mainControl))) {
        setMainForm(form);
        emit mainControlChanged(mainControl);
    }
}

const QList<Form*>& FormScene::forms() const
{
    return _forms;
}
