#ifndef CONTROLEXPOSINGMANAGER_H
#define CONTROLEXPOSINGMANAGER_H

#include <QString>

class Form;
class QPointF;
class Control;
class DesignerScene;

class ControlExposingManager final
{
    Q_DISABLE_COPY(ControlExposingManager)

    friend class InitializationManager;

public:
    static void init(DesignerScene* designerScene);
    static void exposeProject();
    static Form* exposeForm(const QString& rootPath);
    static Control* exposeControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                  Control* parentControl, QString destinationPath,
                                  QString destinationSuid);

private:
    ControlExposingManager() {}

private:
    static DesignerScene* s_designerScene;
};

#endif // CONTROLEXPOSINGMANAGER_H