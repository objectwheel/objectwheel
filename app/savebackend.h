#ifndef SAVEBACKEND_H
#define SAVEBACKEND_H

#include <QObject>

class Form;
class Control;

class SaveBackend : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SaveBackend)

public:
    static SaveBackend* instance();

    bool initProject(const QString& projectDirectory, int templateNumber) const;
    void flushId(const Control* control) const;
    void flushSuid(const Control* control, const QString& suid) const;

    bool existsInForms(const Control* control) const;
    bool existsInFormScope(const Control* control) const;
    bool existsInParentScope(const Control* control, const QString& suid, const QString topPath) const;

    void recalculateUids(Control* control) const;
    void refreshToolUid(const QString& toolRootPath) const;
    void refactorId(Control* control, const QString& suid, const QString& topPath = QString()) const;
    void fixIdConflicts() const;

    bool addForm(Form* form) const;
    bool isInOwdb(const QString& path) const;
    bool isForm(const QString& rootPath) const;
    bool moveControl(Control* control, const Control* parentControl) const;
    bool exists(const Control* control, const QString& suid, const QString& topPath = QString()) const;
    bool addControl(Control* control, const Control* parentControl, const QString& suid, const QString& topPath = QString()) const;

    void removeForm(const Form* form) const;
    void removeControl(const Control* control) const;
    void removeChildControlsOnly(const Control* control) const;
    void removeProperty(const Control* control, const QString& property) const;
    void setProperty(Control* control, const QString& property, QString value, const QString& topPath = QString()) const;

    QString basePath() const;
    QString parentDir(const Control* control) const;
    QString findByUid(const QString& uid, const QString& rootPath) const;
    QString findById(const QString& suid, const QString& id, const QString& rootPath) const;
    QString pathOfId(const QString& suid, const QString& id, const QString& rootPath = QString()) const;
    QStringList formScopePaths() const;

signals:
    void databaseChanged() const;
    void propertyChanged(Control* control, const QString& property, const QString& value) const;

private:
    SaveBackend() {}
};

#endif // SAVEBACKEND_H
