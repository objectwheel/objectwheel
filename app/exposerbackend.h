#ifndef EXPOSERBACKEND_H
#define EXPOSERBACKEND_H

#include <QString>

class Form;
class QPointF;
class Control;
class DesignerScene;

class ExposerBackend
{
        Q_DISABLE_COPY(ExposerBackend)

    public:
        static ExposerBackend* instance();
        void init(DesignerScene* designerScene);
        void exposeProject() const;
        Form* exposeForm(const QString& rootPath) const;
        Control* exposeControl(const QString& rootPath, const QPointF& pos, QString sourceSuid, Control* parentControl, QString destinationPath, QString destinationSuid) const;

    private:
        ExposerBackend();

    private:
        DesignerScene* m_designerScene;
};

#endif // EXPOSERBACKEND_H