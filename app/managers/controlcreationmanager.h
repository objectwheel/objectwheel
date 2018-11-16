#ifndef CONTROLCREATIONMANAGER_H
#define CONTROLCREATIONMANAGER_H

#include <QObject>

class Form;
class QPointF;
class Control;
class DesignerScene;

class ControlCreationManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlCreationManager)

    friend class ApplicationCore; // For construction
    friend class FormsPane; // For createForm

public:
    static ControlCreationManager* instance();
    static Control* createControl(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos);
signals:
    void controlCreated(Control* control); // NOTE: Only used by InspectorPane, so implemention depends

private:
    explicit ControlCreationManager(QObject* parent = nullptr);
    ~ControlCreationManager();

    static void init(DesignerScene* designerScene);
    static Form* createForm(const QString& formRootPath); // FormsPane dependency: Should be a private member

private:
    static ControlCreationManager* s_instance;
    static DesignerScene* s_designerScene;
};

#endif // CONTROLCREATIONMANAGER_H