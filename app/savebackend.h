#ifndef SAVEBACKEND_H
#define SAVEBACKEND_H

#include <QObject>

class Form;
class Control;

class SaveBackend final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SaveBackend)

    friend class BackendManager;

public:
    static SaveBackend* instance();

    static bool initProject(const QString& projectDirectory, int templateNumber);
    static void flushId(const Control* control);
    static void flushSuid(const Control* control, const QString& suid);

    static bool existsInForms(const Control* control);
    static bool existsInFormScope(const Control* control);
    static bool existsInParentScope(const Control* control, const QString& suid, const QString topPath);

    static void recalculateUids(Control* control);
    static void refreshToolUid(const QString& toolRootPath);
    static void refactorId(Control* control, const QString& suid, const QString& topPath = QString());
    static void fixIdConflicts();

    static bool addForm(Form* form);
    static bool isInOwdb(const QString& path);
    static bool isForm(const QString& rootPath);
    static bool moveControl(Control* control, const Control* parentControl);
    static bool exists(const Control* control, const QString& suid, const QString& topPath = QString());
    static bool addControl(Control* control, const Control* parentControl, const QString& suid, const QString& topPath = QString());

    static void removeForm(const Form* form);
    static void removeControl(const Control* control);
    static void removeChildControlsOnly(const Control* control);
    static void removeProperty(const Control* control, const QString& property);
    static void setProperty(Control* control, const QString& property, QString value, const QString& topPath = QString());

    static QString basePath();
    static QString parentDir(const Control* control);
    static QString findByUid(const QString& uid, const QString& rootPath);
    static QString findById(const QString& suid, const QString& id, const QString& rootPath);
    static QString pathOfId(const QString& suid, const QString& id, const QString& rootPath = QString());
    static QStringList formScopePaths();

signals:
    void databaseChanged();
    void propertyChanged(Control* control, const QString& property, const QString& value);

private:
    explicit SaveBackend(QObject* parent = nullptr);
    ~SaveBackend();

private:
    static SaveBackend* s_instance;
};

#endif // SAVEBACKEND_H
