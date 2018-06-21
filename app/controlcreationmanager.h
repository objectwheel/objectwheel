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

    friend class ApplicationCore;

public:
    static ControlCreationManager* instance();
    static void init(DesignerScene* designerScene);
    static Form* createForm(const QString& rootPath);
    static Control* createControl(const QString& rootPath, const QPointF& pos, QString sourceSuid,
                                  Control* parentControl, QString destinationPath,
                                  QString destinationSuid);
signals:
    void formCreated(Form* form);
    void controlCreated(Control* control);

private:
    explicit ControlCreationManager(QObject* parent = nullptr);
    ~ControlCreationManager();

private:
    static ControlCreationManager* s_instance;
    static DesignerScene* s_designerScene;
};

#endif // CONTROLCREATIONMANAGER_H