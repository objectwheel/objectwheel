#ifndef CONTROLPRODUCTIONMANAGER_H
#define CONTROLPRODUCTIONMANAGER_H

#include <QObject>
#include <QImage>

#include <paintutils.h>
#include <controlrenderingmanager.h>

class Form;
class QPointF;
class Control;
class DesignerScene;

class ControlProductionManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlProductionManager)

    friend class ApplicationCore; // For construction
    friend class FormsController; // For produceForm
    friend class DesignerController; // For produceControl
    friend class DesignerScene; // For produceControl

public:
    static ControlProductionManager* instance();

private:
    explicit ControlProductionManager(QObject* parent = nullptr);
    ~ControlProductionManager() override;

    static void init(DesignerScene* designerScene);
    static Form* produceForm(const QString& formRootPath, const QString& module); // FormsController dependency: Should be a private member
    static Control* produceControl(Control* targetParentControl,
                                  const QString& controlRootPath,
                                  const QString& module,
                                  const QPointF& pos,
                                  const QSizeF& initialSize,
                                  const QPixmap& initialPixmap);

signals:
    void controlProduced(Control* control);

private:
    static ControlProductionManager* s_instance;
    static DesignerScene* s_designerScene;
};

#endif // CONTROLPRODUCTIONMANAGER_H