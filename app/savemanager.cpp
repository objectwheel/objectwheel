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

    for (auto path : fps(FILE_PROPERTIES, baseDir)) {
        auto propertyData = rdfile(path);

        if (!SaveUtils::isOwctrl(propertyData))
            continue;

        auto _uid = SaveUtils::property(propertyData, TAG_UID).toString();

        if (_uid == uid)
            return dname(dname(path));
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

    return dname(dname(control->dir()));
}

void flushId(const Control* control)
{
    auto propertyPath = control->dir() + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    SaveUtils::setProperty(propertyData, TAG_ID, control->id());
    wrfile(propertyPath, propertyData);

    auto qmlPath = control->dir() + separator() + DIR_THIS +
                   separator() + "main.qml";
    ParserUtils::setProperty(qmlPath, "id", control->id());
}

void flushSuid(const Control* control, const QString& suid)
{
    auto topPath = control->dir();
    auto fromUid = SaveUtils::suid(topPath);
    if (!fromUid.isEmpty()) {
        for (auto path : fps(FILE_PROPERTIES, topPath)) {
            if (SaveUtils::suid(dname(dname(path))) == fromUid) {
                auto propertyData = rdfile(path);
                auto jobj = QJsonDocument::fromJson(propertyData).object();
                jobj[TAG_SUID] = suid;
                propertyData = QJsonDocument(jobj).toJson();
                wrfile(path, propertyData);
            }
        }
    } else {
        auto propertyPath = control->dir() + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        SaveUtils::setProperty(propertyData, TAG_SUID, suid);
        wrfile(propertyPath, propertyData);
    }
}

// Searches by id.
// Searches control only in forms (in current project)
// If current project is empty, then returns false.
bool existsInForms(const Control* control)
{
    for (auto path : SaveUtils::formPaths(ProjectManager::dir())) {
        auto propertyData = rdfile(path + separator() + DIR_THIS + separator() + FILE_PROPERTIES);
        auto id = SaveUtils::property(propertyData, TAG_ID).toString();

        if (id == control->id())
            return true;
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
bool existsInFormScope(const Control* control)
{
    Q_ASSERT(control->form());
    for (auto path : formScopePaths()) {
        auto propertyData = rdfile(path + separator() + DIR_THIS + separator() + FILE_PROPERTIES);
        auto id = SaveUtils::property(propertyData, TAG_ID).toString();

        if (id == control->id())
            return true;
    }

    return false;
}

// Searches by id.
// Searches control within given suid, search starts within topPath
// Given suid has to be valid
bool existsInParentScope(const Control* control, const QString& suid, const QString topPath)
{
    if (control->form())
        return existsInFormScope(control);

    Q_ASSERT(!suid.isEmpty());

    auto parentRootPath = findByUid(suid, topPath);
    if (parentRootPath.isEmpty())
        return false;
    if (isForm(parentRootPath)) {
        if (existsInForms(control))
            return true;

        for (auto path :  SaveUtils::childrenPaths(parentRootPath)) {
            auto propertyData = rdfile(path + separator() + DIR_THIS +
                                       separator() + FILE_PROPERTIES);
            auto id = SaveUtils::property(propertyData, TAG_ID).toString();

            if (id == control->id())
                return true;
        }

        return false;
    } else {
        QStringList paths(parentRootPath + separator() + DIR_THIS);
        paths << SaveUtils::childrenPaths(parentRootPath);

        for (auto path : paths) {
            auto propertyData = rdfile(path + separator() + DIR_THIS +
                                       separator() + FILE_PROPERTIES);
            auto id = SaveUtils::property(propertyData, TAG_ID).toString();

            if (id == control->id())
                return true;
        }

        return false;
    }
}

// You have to provide an valid suid, except if control is a form
// If topPath is empty, then top level project directory searched
// So, suid and topPath have to be in a valid logical relationship.
bool exists(const Control* control, const QString& suid, const QString& topPath = QString())
{
    return control->form() ? existsInFormScope(control) :
      existsInParentScope(control, suid, topPath);
}

// Recalculates all uids belongs to given control and its children (all).
// Both database and in-memory data are updated.
void recalculateUids(Control* control)
{
    if (control->dir().isEmpty())
        return;

    SaveUtils::recalculateUids(control->dir());

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

    for (int i = 1; exists(control, suid, topPath); i++)
        control->setId(id + QString::number(i));
}
}

SaveManager::SaveManager(QObject* parent) : QObject(parent)
{
}

bool SaveManager::initProject(const QString& projectDirectory, int templateNumber)
{
    if (projectDirectory.isEmpty() ||
        !::exists(projectDirectory) ||
        ::exists(projectDirectory + separator() + DIR_OWDB) ||
        !Zipper::extractZip(rdfile(tr(":/templates/template%1.zip").arg(templateNumber)), projectDirectory))
        return false;

    SaveUtils::recalculateUids(projectDirectory + separator() + DIR_OWDB);

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

    form->setUrl(formDir + separator() + DIR_THIS + separator() + "main.qml");

    flushId(form);
    recalculateUids(form);
    ControlPropertyManager::setId(form, form->id(), false, false);

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
    control->setUrl(controlDir + separator() + DIR_THIS + separator() + "main.qml");

    flushId(control);
    for (auto child : control->childControls())
        flushId(child);

    flushSuid(control, suid);
    recalculateUids(control); //for all

    ControlPropertyManager::setId(control, control->id(), false, false);
    for (auto child : control->childControls())
        ControlPropertyManager::setId(child, child->id(), false, false);

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
    control->setUrl(controlDir + separator() + DIR_THIS + separator() + "main.qml");

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

void SaveManager::refreshToolUid(const QString& toolRootPath)
{
    if (toolRootPath.isEmpty())
        return;

    QStringList paths, properties;

    properties << fps(FILE_PROPERTIES, toolRootPath);
    paths << properties;

    for (auto pfile : properties) {
        auto propertyData = rdfile(pfile);

        if (!SaveUtils::isOwctrl(propertyData))
            continue;

        auto uid = SaveUtils::property(propertyData, TAG_UID).toString();
        auto newUid = HashFactory::generate();

        for (auto file : paths)
            SaveUtils::updateFile(file, uid, newUid);
    }
}

// NOTE: Do not use this directly from anywhere, use ControlPropertyManager instead
// You can not set id property of a top control if it's not exist in the project database
// If you want to set id property of a control that is not exist in the project database,
// then you have to provide a valid topPath
// If topPath is empty, then top level project directory searched
// So, suid and topPath have to be in a valid logical relationship.
// topPath is only necessary if property is an "id" set.
void SaveManager::setProperty(Control* control, const QString& property, QString value, const QString& topPath)
{
    if (!control)
        return;

    if (control->dir().isEmpty())
        return;

    if (!SaveUtils::isOwctrl(control->dir()))
        return;

    if (property == TAG_X) {

    } else if (property == TAG_Y) {

    } else if (property == TAG_Z) {

    } else if (property == TAG_WIDTH) {

    } else if (property == TAG_HEIGHT) {

    } else if (property == TAG_ID) {
        if (control->id() == value)
            return;

        control->setId(value);
        refactorId(control, SaveUtils::suid(control->dir()), topPath);
        // NOTE: We don't have to call ControlPropertyManager::setId in order to emit idChanged signal in
        // ControlPropertyManager; because setProperty cannot be used anywhere else except ControlPropertyManager
        // Hence, setProperty(.., "id", ...) is only called from ControlPropertyManager::setId, which means
        // idChanged signal is already emitted.

        const QString& propertyPath = control->dir() + separator() + DIR_THIS + separator() + FILE_PROPERTIES;
        const QByteArray& propertyData = rdfile(propertyPath);

        SaveUtils::setProperty(propertyData, TAG_ID, QJsonValue(control->id()));
        wrfile(propertyPath, propertyData);

        value = control->id();
    }

    ParserUtils::setProperty(control->url(), property, value);
}

void SaveManager::removeProperty(const Control* control, const QString& property)
{
    if (control->dir().isEmpty() ||
        control->hasErrors() ||
        !SaveUtils::isOwctrl(control->dir()) ||
        property == TAG_ID)
        return;

//    TODO: auto fileName = control->dir() + separator() + DIR_THIS +
//                    separator() + "main.qml";

//    TODO: ParserController::removeVariantProperty(fileName, property);
}