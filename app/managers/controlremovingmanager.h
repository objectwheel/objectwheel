#ifndef CONTROLREMOVINGMANAGER_H
#define CONTROLREMOVINGMANAGER_H

#include <QObject>

class Form;
class Control;
class DesignerScene;

class ControlRemovingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlRemovingManager)

    friend class ApplicationCore; // For construction
    friend class FormsPane; // For removeForm

public:
    static ControlRemovingManager* instance();
    static void removeControl(Control* control);
    static void removeControls(const QList<Control*>& controls);

signals:
    void formAboutToBeRemoved(Form* form);
    void controlAboutToBeRemoved(Control* control);

private:
    explicit ControlRemovingManager(QObject* parent = nullptr);
    ~ControlRemovingManager() override;

    static void init(DesignerScene* designerScene);
    static void removeForm(Form* form);  // FormsPane dependency: Should be a private member

private:
    static ControlRemovingManager* s_instance;
    static DesignerScene* s_designerScene;
};


#endif // CONTROLREMOVINGMANAGER_H