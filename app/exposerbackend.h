#ifndef EXPOSERBACKEND_H
#define EXPOSERBACKEND_H

#include <QString>

class Form;
class QPointF;
class Control;
class DesignerScene;

class ExposerBackend final
{
    Q_DISABLE_COPY(ExposerBackend)

    friend class BackendManager;

public:
    static void init(DesignerScene* designerScene);
    static void exposeProject();
    static Form* exposeForm(const QString& rootPath);
    static Control* exposeControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                  Control* parentControl, QString destinationPath,
                                  QString destinationSuid);

private:
    ExposerBackend() {}

private:
    static DesignerScene* s_designerScene;
};

#endif // EXPOSERBACKEND_H