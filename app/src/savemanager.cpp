#include <savemanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <parsercontroller.h>
#include <formswidget.h>
#include <mainwindow.h>
#include <bindingwidget.h>
#include <eventswidget.h>
#include <qmleditor.h>
#include <eventswidget.h>
#include <bindingwidget.h>
#include <control.h>

#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QTimer>

//!
//! ******************* [SaveManagerPrivate] *******************
//!

class SaveManagerPrivate : public QObject
{
        Q_OBJECT
        // None of these functions checks whether given control's id is valid or not.

	public:
        SaveManagerPrivate(SaveManager* parent);
        bool isOwdb(const QByteArray& propertyData) const;
        bool existsForm(const Form* form) const; // Searches by id.
        bool existsControl(const Control* form) const; // Searches by id.

        // Searches in current project dir.
        // Returns paths of main.qml files (DIR_THIS).
        QStringList formPaths() const;

        // Returns paths of main.qml files (DIR_THIS).
        // Searches in current project dir if basePath is empty.
        // Base path cannot be DIR_THIS of any control.
        QStringList controlPaths(QString& basePath = QString()) const;

        // Returns biggest nubmer from integer named dirs.
        // If no integer named dir exists, 0 returned.
        // If no dir exists or dirs are smaller than zero, 0 returned.
        int biggestDir(const QString& basePath) const;

        // Builds and updates necessary files and folders for a control.
        // Control dir has to be the root dir of control.
        // It doesn't check whether control already exists or not in the database.
        bool buildControlSkeleton(const Control* control, const QString& controlDir) const;

	public:
        SaveManager* parent = nullptr;
        ParserController parserController;
};

SaveManagerPrivate::SaveManagerPrivate(SaveManager* parent)
    : QObject(parent)
    , parent(parent)
{
}

bool SaveManagerPrivate::isOwdb(const QByteArray& propertyData) const
{
    if (propertyData.isEmpty())
        return false;

    auto jobj = QJsonDocument::fromJson(propertyData).object();
    auto sign = jobj[TAG_OWDB_SIGN].toString();

    return (sign == SIGN_OWDB);
}

bool SaveManagerPrivate::existsForm(const Form* form) const
{
    for (auto path : formPaths()) {
        auto propertyFile = rdfile(path + separator() + FILE_PROPERTIES);
        auto jobj = QJsonDocument::fromJson(propertyFile).object();
        auto id = jobj[TAG_ID].toString();
        return (id == form->id());
    }

    return false;
}

bool SaveManagerPrivate::existsControl(const Control* form) const
{
    for (auto path : controlPaths()) {
        auto propertyFile = rdfile(path + separator() + FILE_PROPERTIES);
        auto jobj = QJsonDocument::fromJson(propertyFile).object();
        auto id = jobj[TAG_ID].toString();
        return (id == form->id());
    }

    return false;
}

QStringList SaveManagerPrivate::formPaths() const
{
    QStringList paths;
    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

    if (projectDir.isEmpty())
        return paths;

    auto baseDir = projectDirectory + separator() + DIR_FORMS;

    for (auto dir : lsdir(baseDir)) {
        auto propertyPath = baseDir + separator() + dir + separator() +
                            DIR_THIS + separator() + FILE_PROPERTIES;
        auto propertyFile = rdfile(propertyPath);

        if (isOwdb(propertyFile))
            paths << dname(propertyPath);
    }

    return paths;
}

QStringList SaveManagerPrivate::controlPaths(QString& basePath) const
{
    // TODO:
}

int SaveManagerPrivate::biggestDir(const QString& basePath) const
{
    int num = 0;
    for (auto dir : lsdir(basePath))
        if (dir.toInt() > num)
            num = dir.toInt();
    return num;
}

bool SaveManagerPrivate::buildControlSkeleton(const Control* control, const QString& controlDir) const
{
    if (!(mkdir(controlDir) &&
          mkdir(controlDir + separator() + DIR_THIS) &&
          mkdir(controlDir + separator() + DIR_CHILDREN) &&
          mkfile(controlDir + separator() + DIR_THIS + separator() + FILE_PROPERTIES) &&
          mkfile(controlDir + separator() + DIR_THIS + separator() + FILE_EVENTS)
          /* TODO: Add mkfile bindings */ ))
        return false;

    auto propertyData = rdfile(controlDir + separator() + DIR_THIS + separator() + FILE_PROPERTIES);
    QJsonDocument jdoc(QJsonDocument::fromJson(propertyData));
    QJsonObject jobj(jdoc.object());
    jobj[TAG_OWDB_SIGN] = SIGN_OWDB;
    jobj[TAG_ID] = control->id();
    return (wrfile(controlDir + separator() + DIR_THIS + separator() + FILE_PROPERTIES, jdoc.toJson()) > 0);
}

//!
//! ********************** [SaveManager] **********************
//!

SaveManagerPrivate* SaveManager::_d = nullptr;

SaveManager::SaveManager(QObject *parent)
	: QObject(parent)
{
    if (_d)
        return;
    _d = new SaveManagerPrivate(this);
}

SaveManager* SaveManager::instance()
{
    return _d->parent;
}

bool SaveManager::execProject()
{

}

bool SaveManager::exposeProject()
{

}

Control* SaveManager::exposeControl(const QString& basePath)
{

}

bool SaveManager::exists(const Control* control)
{
    return control->form() ? _d->existsForm(control) : _d->existsControl(control);
}

void SaveManager::addForm(Form* form)
{
    if (form->id().isEmpty())
        return;

    if (exists(form))
        return;

    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

    if (projectDir.isEmpty())
        return;

    auto baseDir = projectDirectory + separator() + DIR_FORMS;

    if (!_d->buildControlSkeleton(form, baseDir))
        return;

    auto formDir = baseDir + separator() + QString::number(biggestDir(baseDir));
    form->setDir(formDir + separator() + DIR_THIS);

    return cp(dname(form->url()), formDir + separator() + DIR_THIS, true);
}

void SaveManager::addControl(Control* control, const Control* parentControl)
{
    if (control->id().isEmpty())
        return;

    if (exists(control))
        return;

    if (parentControl->dir().isEmpty())
        return;

    auto baseDir = dname(parentControl->dir()) + separator() + DIR_CHILDREN;
    auto controlDir = baseDir + separator() + QString::number(biggestDir(baseDir) + 1);

    if (!mkdir(controlDir))
        return;

    if (!cp(dname(dname(control->url())), controlDir, true))
        return;

    if (!_d->buildControlSkeleton(control, controlDir))
        return;

    control->setDir(controlDir + separator() + DIR_THIS);

    return true;
}

void SaveManager::setVariantProperty(const QString& id, const QString& property, const QVariant& value)
{
    if (saveDirectory(id).isEmpty()) return;
    auto filename = saveDirectory(id) + separator() + "main.qml";
    ParserController::setVariantProperty(filename, property, value);
    QmlEditor::clearCacheFor(saveDirectory(id), true);
}

void SaveManager::removeVariantProperty(const QString& id, const QString& property) //FIXME: FOR BINDING PROPERTIES
{
    if (saveDirectory(id).isEmpty()) return;
    auto filename = saveDirectory(id) + separator() + "main.qml";
    ParserController::removeVariantProperty(filename, property);
}

bool SaveManager::inprogress()
{
    return _d->parserController.running();
}

