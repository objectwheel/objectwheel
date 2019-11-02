#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

class Control;

class SaveManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SaveManager)

    friend class ApplicationCore;        // For constructor and destructor
    friend class ControlRemovingManager; // For removeControl()
    friend class ControlCreationManager; // For addControl()
    friend class ControlPropertyManager; // For setProperty(), setIndex(), moveControl()
    friend class ProjectManager;         // For initProject()

public:
    static SaveManager* instance();

private:
    explicit SaveManager(QObject* parent = nullptr);
    ~SaveManager() override;

    static QString addForm(const QString& formRootPath);
    static void removeForm(const QString& formRootPath);
    static void setupFormConnections(const QString& formRootPath);

    static QString addControl(const QString& controlRootPath, const QString& targetParentControlRootPath);
    static bool moveControl(Control* control, const Control* parentControl);
    static void removeControl(const QString& rootPath);

    static void setIndex(Control* control, quint32 index);
    static void setProperty(Control* control, const QString& property, const QString& value);

signals:
    void formConnectionsDone(const QString& FormJS, const QString& id);
    void propertyChanged(Control* control, const QString& property, const QString& value);

private:
    static SaveManager* s_instance;
};

#endif // SAVEMANAGER_H
