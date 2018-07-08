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

private:
    explicit SaveManager(QObject* parent = nullptr);

    static bool initProject(const QString& projectDirectory, int templateNumber);

    static void setProperty(Control* control, const QString& property, QString value, const QString& topPath = QString());
    static void removeProperty(const Control* control, const QString& property);

    static bool addForm(Form* form);
    static void removeForm(const Form* form);

    static bool addControl(Control* control, const Control* parentControl, const QString& suid, const QString& topPath = QString());
    static bool moveControl(Control* control, const Control* parentControl);
    static void removeControl(const Control* control);
};

#endif // SAVEMANAGER_H
