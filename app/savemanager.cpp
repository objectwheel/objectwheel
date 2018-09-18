#include <savemanager.h>
#include <saveutils.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <form.h>
#include <parserutils.h>
#include <hashfactory.h>
#include <zipper.h>
#include <controlpropertymanager.h>

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace {
// Returns true if given root path belongs to a form
// Searches within current project's path
bool isForm(const QString& rootPath)
{
    auto projectDir = ProjectManager::dir();
    auto baseDir = projectDir + separator() + DIR_OWDB;
    return (baseDir == dname(rootPath));
}

// Returns root path if given uid belongs to a control
// Searches within given rootPath (root control included)
// Searches in current project directory if given rootPath is empty
// All controls in the given rootPath are included to search
QString findByUid(const QString& uid, const QString& rootPath)
{
    QString baseDir;
    if (rootPath.isEmpty()) {
        auto projectDir = ProjectManager::dir();

        if (projectDir.isEmpty())
            return QString();

        baseDir = projectDir;
    } else {
        baseDir = rootPath;
    }

    for (auto path : fps(FILE_CONTROL, baseDir)) {
        if (!SaveUtils::isOwctrl(SaveUtils::toParentDir(path)))
            continue;

        auto _uid = SaveUtils::uid(SaveUtils::toParentDir(path));

        if (_uid == uid)
            return SaveUtils::toParentDir(path);
    }

    return QString();
}

// Returns root path (of parent) if given control has a parent control
QString parentDir(const Control* control)
{
    if (control->form() ||
            control->dir().isEmpty() ||
            !SaveUtils::isOwctrl(control->dir()))
        return QString();

    return SaveUtils::toParentDir(control->dir());
}

void flushSuid(const Control* control, const QString& suid)
{
    auto topPath = control->dir();
    auto fromUid = SaveUtils::suid(topPath);
    if (!fromUid.isEmpty()) {
        for (auto path : fps(FILE_CONTROL, topPath)) {
            if (SaveUtils::suid(SaveUtils::toParentDir(path)) == fromUid)
                SaveUtils::setProperty(SaveUtils::toParentDir(path), TAG_SUID, suid);
        }
    } else {
        SaveUtils::setProperty(control->dir(), TAG_SUID, suid);
    }
}

// Searches by id.
// Searches control only in forms (in current project)
// If current project is empty, then returns false.
bool existsInForms(const Control* control, bool includeItself)
{
    for (auto path : SaveUtils::formPaths(ProjectManager::dir())) {
        if ((includeItself || path != control->dir())
                && ParserUtils::id(SaveUtils::toUrl(path)) == control->id()) {
            return true;
        }
    }

    return false;
}

// Returns all control paths in form scope.
// Returned paths are rootPaths.
QStringList formScopePaths()
{
    const QStringList fpaths = SaveUtils::formPaths(ProjectManager::dir());
    QStringList paths(fpaths);

    for (auto path : fpaths)
        paths << SaveUtils::childrenPaths(dname(path));

    return paths;
}

// Searches by id.
// Searches control in form scope (in current project)
// If current project is empty, then returns false.
bool existsInFormScope(const Control* control, bool includeItself)
{
    Q_ASSERT(control->form());
    for (auto path : formScopePaths()) {
        if ((includeItself || path != control->dir())
                && ParserUtils::id(SaveUtils::toUrl(path)) == control->id()) {
            return true;
        }
    }

    return false;
}

// Searches by id.
// Searches control within given suid, search starts within topPath
// Given suid has to be valid
bool existsInParentScope(const Control* control, const QString& suid, const QString topPath, bool includeItself)
{
    if (control->form())
        return existsInFormScope(control, includeItself);

    Q_ASSERT(!suid.isEmpty());

    auto parentRootPath = findByUid(suid, topPath);
    if (parentRootPath.isEmpty())
        return false;

    Q_ASSERT(parentRootPath != control->dir());

    if (isForm(parentRootPath)) {
        if (existsInForms(control, includeItself))
            return true;

        for (auto path :  SaveUtils::childrenPaths(parentRootPath)) {
            if ((includeItself || path != control->dir())
                    && ParserUtils::id(SaveUtils::toUrl(path)) == control->id()) {
                return true;
            }
        }

        return false;
    } else {
        if (ParserUtils::id(SaveUtils::toUrl(parentRootPath)) == control->id())
            return true;

        for (auto path : SaveUtils::childrenPaths(parentRootPath)) {
            if ((includeItself || path != control->dir())
                    && ParserUtils::id(SaveUtils::toUrl(path)) == control->id()) {
                return true;
            }
        }

        return false;
    }
}

// You have to provide an valid suid, except if control is a form
// If topPath is empty, then top level project directory searched
// So, suid and topPath have to be in a valid logical relationship.
bool exists(const Control* control, const QString& suid, const QString& topPath = QString(),
            bool includeItself = true)
{
    return control->form() ? existsInFormScope(control, includeItself) :
                             existsInParentScope(control, suid, topPath, includeItself);
}

// Recalculates all uids belongs to given control and its children (all).
// Both database and in-memory data are updated.
void regenerateUids(Control* control)
{
    if (control->dir().isEmpty())
        return;

    SaveUtils::regenerateUids(control->dir());

    Control::updateUids(); // FIXME: Change with childControls()->updateUid();
}

// NOTE: If you use refactorId, then don't forget to call ControlPropertyManager::setId
// in order to emit idChanged signal in ControlPropertyManager
// Refactor control's id if it's already exists in db
// If suid empty, project root searched
void refactorId(Control* control, const QString& suid, const QString& topPath = QString())
{
    if (control->id().isEmpty())
        control->setId("control");

    const auto id = control->id();

    for (int i = 1; exists(control, suid, topPath, false); i++)
        control->setId(id + QString::number(i));
}
}

SaveManager* SaveManager::s_instance = nullptr;
SaveManager::SaveManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

SaveManager::~SaveManager()
{
    s_instance = nullptr;
}

SaveManager* SaveManager::instance()
{
    return s_instance;
}

bool SaveManager::initProject(const QString& projectDirectory, int templateNumber)
{
    if (projectDirectory.isEmpty() ||
            !::exists(projectDirectory) ||
            ::exists(projectDirectory + separator() + DIR_OWDB) ||
            !Zipper::extractZip(rdfile(tr(":/templates/template%1.zip").arg(templateNumber)), projectDirectory))
        return false;

    SaveUtils::regenerateUids(projectDirectory + separator() + DIR_OWDB);

    return true;
}

bool SaveManager::addForm(Form* form)
{
    if (form->url().isEmpty())
        return false;

    if (!SaveUtils::isOwctrl(form->dir()))
        return false;

    // NOTE: We don't have to worry about possible child controls since addForm is only
    // called from FormsPane > ControlCreationManager
    refactorId(form, QString());

    auto projectDir = ProjectManager::dir();

    if (projectDir.isEmpty())
        return false;

    auto baseDir = projectDir + separator() + DIR_OWDB;
    auto formDir = baseDir + separator() + QString::number(SaveUtils::biggestDir(baseDir) + 1);

    if (!mkdir(formDir))
        return false;

    if (!cp(form->dir(), formDir, true))
        return false;

    form->setUrl(SaveUtils::toUrl(formDir));

    ParserUtils::setId(form->url(), form->id());
    regenerateUids(form);
    ControlPropertyManager::setId(form, form->id(), ControlPropertyManager::NoOption);

    return true;
}

void SaveManager::removeForm(const Form* form)
{
    if (form->id().isEmpty() || form->url().isEmpty())
        return;

    if (form->main() || !SaveUtils::isOwctrl(form->dir()) || !exists(form, QString()))
        return;

    rm(form->dir());
}

void SaveManager::setupFormGlobalConnections(Form* form)
{
    Q_ASSERT(!form->id().isEmpty() && !form->url().isEmpty());

    QString id = form->id();
    const QString FormJS = id.replace(0, 1, id[0].toUpper()) + "JS";
    id = form->id();

    QString content = rdfile(form->url());
    Q_ASSERT(!content.isEmpty());

    content.replace(QString::fromUtf8("// GlobalConnectionHere"),
                    QString::fromUtf8("Component.onCompleted: %1.%2_onCompleted()").arg(FormJS).arg(id));

    wrfile(form->url(), content.toUtf8());

    const QString& globalJSPath = SaveUtils::toGlobalDir(ProjectManager::dir()) + separator() + id + ".js";
    QString js = rdfile(":/resources/other/form.js");
    js = js.arg(id);
    if (!exists(globalJSPath))
        wrfile(globalJSPath, js.toUtf8());
    else
        return (void) (qWarning() << tr("SaveManager::setupFormGlobalConnections: Global %1 file is already exists.").arg(id + ".js"));

    emit instance()->formGlobalConnectionsDone(FormJS, id);
}

bool SaveManager::addControl(Control* control, const Control* parentControl, const QString& suid, const QString& topPath)
{
    if (control->url().isEmpty())
        return false;

    if (parentControl->dir().isEmpty())
        return false;

    if (!SaveUtils::isOwctrl(control->dir()) || !SaveUtils::isOwctrl(parentControl->dir()))
        return false;

    refactorId(control, suid, topPath);
    for (auto child : control->childControls())
        refactorId(child, suid, topPath);

    auto baseDir = parentControl->dir() + separator() + DIR_CHILDREN;
    auto controlDir = baseDir + separator() + QString::number(SaveUtils::biggestDir(baseDir) + 1);

    if (!mkdir(controlDir))
        return false;

    if (!cp(control->dir(), controlDir, true))
        return false;

    for (auto child : control->childControls())
        child->setUrl(child->url().replace(control->dir(), controlDir));
    control->setUrl(SaveUtils::toUrl(controlDir));

    ParserUtils::setId(control->url(), control->id());
    for (auto child : control->childControls())
        ParserUtils::setId(child->url(), child->id());

    flushSuid(control, suid);
    regenerateUids(control); //for all

    ControlPropertyManager::setId(control, control->id(), ControlPropertyManager::NoOption);
    for (auto child : control->childControls())
        ControlPropertyManager::setId(child, child->id(), ControlPropertyManager::NoOption);

    return true;
}

// You can only move controls within current suid scope of related control
bool SaveManager::moveControl(Control* control, const Control* parentControl)
{
    if (parentDir(control) == parentControl->dir())
        return true;

    if (control->id().isEmpty() || control->url().isEmpty())
        return false;

    if (parentControl->url().isEmpty())
        return false;

    if (!SaveUtils::isOwctrl(control->dir()) || !SaveUtils::isOwctrl(parentControl->dir()))
        return false;

    if (!parentControl->form() && SaveUtils::suid(control->dir()) != SaveUtils::suid(parentControl->dir()))
        return false;

    auto baseDir = parentControl->dir() + separator() + DIR_CHILDREN;
    auto controlDir = baseDir + separator() + QString::number(SaveUtils::biggestDir(baseDir) + 1);

    if (!mkdir(controlDir))
        return false;

    if (!cp(control->dir(), controlDir, true))
        return false;

    if (!rm(control->dir()))
        return false;

    for (auto child : control->childControls())
        child->setUrl(child->url().replace(control->dir(), controlDir));
    control->setUrl(SaveUtils::toUrl(controlDir));

    return true;
}

void SaveManager::removeControl(const Control* control)
{
    if (control->id().isEmpty() || control->url().isEmpty())
        return;

    if (!SaveUtils::isOwctrl(control->dir()))
        return;

    rm(control->dir());
}

/*
    NOTE: Do not use this directly from anywhere, use ControlPropertyManager instead
    You can not set id property of a top control if it's not exist in the project database
    If you want to set id property of a control that is not exist in the project database,
    then you have to provide a valid topPath
    If topPath is empty, then top level project directory searched
    So, suid and topPath have to be in a valid logical relationship.
    topPath is only necessary if property is an "id" set.
*/
void SaveManager::setProperty(Control* control, const QString& property, QString value, const QString& topPath)
{
    if (!control)
        return;

    if (control->dir().isEmpty())
        return;

    if (!SaveUtils::isOwctrl(control->dir()))
        return;

    if (property == "id") {
        if (control->id() == value)
            return;

        /*
            NOTE: We don't have to call ControlPropertyManager::setId in order to emit
                  idChanged signal in ControlPropertyManager; because setProperty cannot be
                  used anywhere else except ControlPropertyManager Hence,
                  setProperty(.., "id", ...) is only called from ControlPropertyManager::setId,
                  which means idChanged signal is already emitted.
        */
        control->setId(value);
        refactorId(control, SaveUtils::suid(control->dir()), topPath);
        value = control->id();
        ParserUtils::setId(control->url(), value);
    } else {
        ParserUtils::setProperty(control->url(), property, value);
    }

    instance()->propertyChanged(control, property, value);
}

void SaveManager::removeProperty(const Control* control, const QString& property)
{
    // TODO:
    //    if (control->dir().isEmpty() ||
    //            control->hasErrors() ||
    //            !SaveUtils::isOwctrl(control->dir()) ||
    //            property == TAG_ID)
    //        return;
    //    ParserController::removeVariantProperty(control->url(), property);
}