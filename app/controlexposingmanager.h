#ifndef CONTROLEXPOSINGMANAGER_H
#define CONTROLEXPOSINGMANAGER_H

#include <QObject>

class Form;
class QPointF;
class Control;
class DesignerScene;

class ControlExposingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlExposingManager)

    friend class InitializationManager;

public:
    static ControlExposingManager* instance();
    static void init(DesignerScene* designerScene);
    static void exposeProject();
    static Form* exposeForm(const QString& rootPath);
    static Control* exposeControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                  Control* parentControl, QString destinationPath,
                                  QString destinationSuid);
signals:
    void formExposed(Form* form);
    void controlExposed(Control* control);

private:
    explicit ControlExposingManager(QObject* parent = nullptr);
    ~ControlExposingManager();

private:
    static ControlExposingManager* s_instance;
    static DesignerScene* s_designerScene;
};

#endif // CONTROLEXPOSINGMANAGER_H