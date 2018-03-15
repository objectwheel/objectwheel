#ifndef EXPOSERBACKEND_H
#define EXPOSERBACKEND_H

#include <QString>

class Control;
class DesignerScene;

class ExposerBackend
{
        Q_DISABLE_COPY(ExposerBackend)

    public:
        static ExposerBackend* instance();
        void init(DesignerScene* designerScene);
        void exposeProject() const;
        Control* exposeControl(const QString& rootPath, QString suid = QString()) const;

    private:
        ExposerBackend();

    private:
        DesignerScene* m_designerScene;
};

#endif // EXPOSERBACKEND_H