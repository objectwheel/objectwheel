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

    friend class InitializationManager;

public:
    static ControlRemovingManager* instance();
    static void init(DesignerScene* designerScene);
    static void removeForm(Form* form);
    static void removeControl(Control* control);

signals:
    void formAboutToBeRemoved(Form* form);
    void controlAboutToBeRemoved(Control* control);

private:
    explicit ControlRemovingManager(QObject* parent = nullptr);
    ~ControlRemovingManager();

private:
    static ControlRemovingManager* s_instance;
    static DesignerScene* s_designerScene;
};


#endif // CONTROLREMOVINGMANAGER_H