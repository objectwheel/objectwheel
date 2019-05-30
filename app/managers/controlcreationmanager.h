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
    friend class DesignerWidget; // For createControl
    friend class DesignerView; // For createControl

private:
    explicit ControlCreationManager(QObject* parent = nullptr);

    static void init(DesignerScene* designerScene);
    static Form* createForm(const QString& formRootPath); // FormsPane dependency: Should be a private member
    static Control* createControl(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos); // NOTE: Sets pos, based on scene coord jargon

private:
    static DesignerScene* s_designerScene;
};

#endif // CONTROLCREATIONMANAGER_H