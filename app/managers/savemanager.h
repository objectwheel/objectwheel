#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

class Form;
class Control;

class SaveManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SaveManager)

    friend class ApplicationCore;        // For constructor and destructor
    friend class ControlRemovingManager; // For removeControl()
    friend class ControlCreationManager; // For addControl()
    friend class ControlPropertyManager; // For setProperty(), moveControl()
    friend class ProjectManager;         // For initProject()

public:
    static SaveManager* instance();

private:
    explicit SaveManager(QObject* parent = nullptr);
    ~SaveManager();

    static bool initProject(const QString& projectDirectory, int templateNumber);

    static void setProperty(Control* control, const QString& property, QString value, const QString& topPath = QString());

    static bool addForm(const QString& formRootPath);
    static void removeForm(const QString& formRootPath);
    static void setupFormGlobalConnections(const QString& formRootPath);

    static bool addControl(const QString& controlRootPath, const QString& targetParentControlRootPath, const QString& targetFormRootPath);
    static bool moveControl(Control* control, const Control* parentControl);
    static void removeControl(const QString& rootPath);

private:
    static void repairIdsInProjectFormScope(const QString& rootPath, const QString& formRootPath);

signals:
    void formGlobalConnectionsDone(const QString& FormJS, const QString& id);
    void propertyChanged(Control* control, const QString& property, const QString& value);

private:
    static SaveManager* s_instance;
};

#endif // SAVEMANAGER_H
