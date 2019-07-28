#ifndef CONTROLREMOVINGMANAGER_H
#define CONTROLREMOVINGMANAGER_H

#include <QObject>

class Control;
class DesignerScene;

class ControlRemovingManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlRemovingManager)

    friend class ApplicationCore; // For construction

public:
    static ControlRemovingManager* instance();
    static void removeControl(Control* control, bool removeFromDatabaseAlso);
    static void removeControls(const QList<Control*>& controls, bool removeFromDatabaseAlso);

signals:
    void controlAboutToBeRemoved(Control* control);

private:
    explicit ControlRemovingManager(QObject* parent = nullptr);
    ~ControlRemovingManager() override;
    static void init(DesignerScene* scene);

private:
    static ControlRemovingManager* s_instance;
    static DesignerScene* s_designerScene;
};

#endif // CONTROLREMOVINGMANAGER_H