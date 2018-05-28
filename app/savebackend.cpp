#include <savebackend.h>
#include <saveutils.h>
#include <filemanager.h>
#include <projectbackend.h>
#include <form.h>
#include <parserutils.h>
#include <hashfactory.h>
#include <zipper.h>

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

SaveBackend* SaveBackend::s_instance = nullptr;

SaveBackend::SaveBackend(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

SaveBackend::~SaveBackend()
{
    s_instance = nullptr;
}

SaveBackend* SaveBackend::instance()
{
    return s_instance;
}

bool SaveBackend::initProject(const QString& projectDirectory, int templateNumber)
{
    if (projectDirectory.isEmpty() ||
        !::exists(projectDirectory) ||
        ::exists(projectDirectory + separator() + DIR_OWDB) ||
        !Zipper::extractZip(rdfile(tr(":/templates/template%1.zip").arg(templateNumber)), projectDirectory))
        return false;

    SaveUtils::recalculateUids(projectDirectory + separator() + DIR_OWDB);

    return true;
}

void SaveBackend::flushId(const Control* control)
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

void SaveBackend::flushSuid(const Control* control, const QString& suid)
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
bool SaveBackend::existsInForms(const Control* control)
{
    for (auto path : SaveUtils::formPaths(ProjectBackend::dir())) {
        auto propertyData = rdfile(path + separator() + DIR_THIS + separator() + FILE_PROPERTIES);
        auto id = SaveUtils::property(propertyData, TAG_ID).toString();

        if (id == control->id())
            return true;
    }

    return false;
}

// Searches by id.
// Searches control in form scope (in current project)
// If current project is empty, then returns false.
bool SaveBackend::existsInFormScope(const Control* control)
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
bool SaveBackend::existsInParentScope(const Control* control, const QString& suid, const QString topPath)
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

// Returns all control paths in form scope.
// Returned paths are rootPaths.
QStringList SaveBackend::formScopePaths()
{
    const QStringList fpaths = SaveUtils::formPaths(ProjectBackend::dir());
    QStringList paths(fpaths);

    for (auto path : fpaths)
        paths << SaveUtils::childrenPaths(dname(path));

    return paths;
}

// Recalculates all uids belongs to given control and its children (all).
// Both database and in-memory data are updated.
void SaveBackend::recalculateUids(Control* control)
{
    if (control->dir().isEmpty())
        return;

    SaveUtils::recalculateUids(control->dir());

    Control::updateUids(); // FIXME: Change with childControls()->updateUid();
}

// Refactor control's id if it's already exists in db
// If suid empty, project root searched
void SaveBackend::refactorId(Control* control, const QString& suid, const QString& topPath)
{
    if (control->id().isEmpty())
        control->setId("control");

    const auto id = control->id();

    for (int i = 1; exists(control, suid, topPath); i++)
        control->setId(id + QString::number(i));
}

// Returns true if given root path belongs to a form
// Searches within current project's path
bool SaveBackend::isForm(const QString& rootPath)
{
    auto projectDir = ProjectBackend::dir();
    auto baseDir = projectDir + separator() + DIR_OWDB;
    return (baseDir == dname(rootPath));
}

// Returns root path if given uid belongs to a control
// Searches within given rootPath (root control included)
// Searches in current project directory if given rootPath is empty
// All controls in the given rootPath are included to search
QString SaveBackend::findByUid(const QString& uid, const QString& rootPath)
{
    QString baseDir;
    if (rootPath.isEmpty()) {
        auto projectDir = ProjectBackend::dir();

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

// Returns root path if given id belongs to a control
// Searches within given rootPath (root control included)
// Searches in current project directory if given rootPath is empty
// Only suid scope controls in the given rootPath are returned
QString SaveBackend::findById(const QString& suid, const QString& id, const QString& rootPath)
{
    QString baseDir;
    if (rootPath.isEmpty()) {
        auto projectDir = ProjectBackend::dir();

        if (projectDir.isEmpty())
            return QString();

        baseDir = projectDir;
    } else {
        baseDir = rootPath;
    }

    for (auto path : fps(FILE_PROPERTIES, baseDir)) {
        auto propertyData = rdfile(path);

        if (!SaveUtils::isOwctrl(propertyData) ||
            (SaveUtils::uid(dname(dname(path))) != suid &&
             SaveUtils::suid(dname(dname(path))) != suid))
            continue;

        auto _id = SaveUtils::property(propertyData, TAG_ID).toString();

        if (_id == id)
            return dname(dname(path));
    }

    return QString();
}

// Returns true if given path is inside of owdb
bool SaveBackend::isInOwdb(const QString& path)
{
    auto projectDirectory = ProjectBackend::dir();

    Q_ASSERT(!projectDirectory.isEmpty());

    QString owdbPath = projectDirectory + separator() + DIR_OWDB;
    return path.contains(owdbPath, Qt::CaseInsensitive);
}

// Returns root path (of parent) if given control has a parent control
QString SaveBackend::parentDir(const Control* control)
{
    if (control->form() ||
        control->dir().isEmpty() ||
        !SaveUtils::isOwctrl(control->dir()))
        return QString();

    return dname(dname(control->dir()));
}

QString SaveBackend::basePath()
{
    auto projectDir = ProjectBackend::dir();

    if (projectDir.isEmpty())
        return QString();

    return (projectDir + separator() + DIR_OWDB);
}

void SaveBackend::refreshToolUid(const QString& toolRootPath)
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

// You have to provide an valid suid, except if control is a form
// If topPath is empty, then top level project directory searched
// So, suid and topPath have to be in a valid logical relationship.
bool SaveBackend::exists(const Control* control, const QString& suid, const QString& topPath)
{
    return control->form() ? existsInFormScope(control) :
      existsInParentScope(control, suid, topPath);
}

bool SaveBackend::addForm(Form* form)
{
    if (form->url().isEmpty())
        return false;

    if (!SaveUtils::isOwctrl(form->dir()))
        return false;

    refactorId(form, QString());

    auto projectDir = ProjectBackend::dir();

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

    emit instance()->databaseChanged();

    return true;
}

void SaveBackend::removeForm(const Form* form)
{
    if (form->id().isEmpty() || form->url().isEmpty())
        return;

    if (form->main() || !SaveUtils::isOwctrl(form->dir()) || !exists(form, QString()))
        return;

    rm(form->dir());

    emit instance()->databaseChanged();
}

bool SaveBackend::addControl(Control* control, const Control* parentControl, const QString& suid, const QString& topPath)
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

    if (isInOwdb(control->dir()))
        emit instance()->databaseChanged();

    return true;
}

// You can only move controls within current suid scope of related control
bool SaveBackend::moveControl(Control* control, const Control* parentControl)
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

    control->setUrl(controlDir + separator() + DIR_THIS + separator() + "main.qml");

    if (isInOwdb(control->dir()))
        emit instance()->databaseChanged();

    return true;
}

void SaveBackend::removeControl(const Control* control)
{
    if (control->id().isEmpty() || control->url().isEmpty())
        return;

    if (!SaveUtils::isOwctrl(control->dir()))
        return;

    rm(control->dir());

    if (isInOwdb(control->dir()))
        emit instance()->databaseChanged();
}

void SaveBackend::removeChildControlsOnly(const Control* control)
{
    if (control->id().isEmpty() || control->url().isEmpty())
        return;

    if (!SaveUtils::isOwctrl(control->dir()))
        return;

    rm(control->dir() + separator() + DIR_CHILDREN);

    if (isInOwdb(control->dir()))
        emit instance()->databaseChanged();
}

// You can not set id property of a top control if it's not exist in the project database
// If you want to set id property of a control that is not exist in the project database,
// then you have to provide a valid topPath
// If topPath is empty, then top level project directory searched
// So, suid and topPath have to be in a valid logical relationship.
// topPath is only necessary if property is an "id" set.
void SaveBackend::setProperty(Control* control, const QString& property, QString value, const QString& topPath)
{
    if (control->dir().isEmpty() ||
        !SaveUtils::isOwctrl(control->dir()) ||
        control->hasErrors())
        return;

    if (property == TAG_ID) {
        if (control->id() == value)
            return;

        auto _suid = SaveUtils::suid(control->dir());
        control->setId(value);
        refactorId(control, _suid, topPath);

        auto propertyPath = control->dir() + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        SaveUtils::setProperty(propertyData, TAG_ID, QJsonValue(control->id()));
        wrfile(propertyPath, propertyData);

        value = control->id();
    }

    auto fileName = control->dir() + separator() + DIR_THIS +
                    separator() + "main.qml";
    ParserUtils::setProperty(fileName, property, value);
    emit instance()->propertyChanged(control, property, value);

    if (isInOwdb(control->dir())) //FIXME
        emit instance()->databaseChanged();
}

void SaveBackend::removeProperty(const Control* control, const QString& property)
{
    if (control->dir().isEmpty() ||
        control->hasErrors() ||
        !SaveUtils::isOwctrl(control->dir()) ||
        property == TAG_ID)
        return;

//    auto fileName = control->dir() + separator() + DIR_THIS +
//                    separator() + "main.qml";

//    ParserController::removeVariantProperty(fileName, property);

    if (isInOwdb(control->dir()))
        emit instance()->databaseChanged();
}

QString SaveBackend::pathOfId(const QString& suid, const QString& id, const QString& rootPath)
{
    return findById(suid, id, rootPath);
}
