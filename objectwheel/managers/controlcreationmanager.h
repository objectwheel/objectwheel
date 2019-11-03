#ifndef CONTROLCREATIONMANAGER_H
#define CONTROLCREATIONMANAGER_H

#include <QObject>
#include <QImage>

#include <paintutils.h>
#include <controlrenderingmanager.h>

class Form;
class QPointF;
class Control;
class DesignerScene;

class ControlCreationManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlCreationManager)

    friend class ApplicationCore; // For construction
    friend class FormsController; // For createForm
    friend class DesignerController; // For createControl
    friend class DesignerScene; // For createControl

public:
    static ControlCreationManager* instance();

private:
    explicit ControlCreationManager(QObject* parent = nullptr);
    ~ControlCreationManager() override;

    static void init(DesignerScene* designerScene);
    static Form* createForm(const QString& formRootPath, const QString& module); // FormsController dependency: Should be a private member
    static Control* createControl(Control* targetParentControl,
                                  const QString& controlRootPath,
                                  const QString& module,
                                  const QPointF& pos,
                                  const QSizeF& initialSize,
                                  const QPixmap& initialPixmap);

signals:
    void controlCreated(Control* control);

private:
    static ControlCreationManager* s_instance;
    static DesignerScene* s_designerScene;
};

#endif // CONTROLCREATIONMANAGER_H