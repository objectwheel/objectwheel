#include <savemanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <parsercontroller.h>
#include <formswidget.h>
#include <mainwindow.h>
#include <control.h>
#include <designmanager.h>
#include <algorithm>
#include <delayer.h>

#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QQuickItem>
#include <QQmlProperty>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickView>

//!
//! ******************* [SaveManagerPrivate] *******************
//!

class SaveManagerPrivate : public QObject
{
        Q_OBJECT
        // Forms: All forms (children not included).
        // Form Scope: All forms + Primary children of them.
        // Parent Scope: Parent + Primary children of Parent.
        // None of member functions checks whether given controls' ids or dirs are valid or not.

    public:
        enum Type {
            Quick,
            Window,
            NonGui
        };

    public:
        SaveManagerPrivate(SaveManager* parent);
        void setProperty(QByteArray& propertyData, const QString& property, const QJsonValue& value) const;
        QJsonValue property(const QByteArray& propertyData, const QString& property) const;
        void flushId(const Control* control, const QString& id) const;
        void flushSuid(const Control* control, const QString& suid) const;
        bool isOwctrl(const QByteArray& propertyData) const;

        // Searches by id.
        // Searches control only in forms (in current project)
        // If current project is empty, then returns false.
        bool existsInForms(const Control* control) const;

        // Searches by id.
        // Searches control in form scope (in current project)
        // If current project is empty, then returns false.
        bool existsInFormScope(const Control* control) const;

        // Searches by id.
        // Searches control within given suid, search starts within topPath
        // Given suid has to be valid
        bool existsInParentScope(const Control* control, const QString& suid, const QString topPath) const;

        // Searches for all controls paths, starting from topPath.
        // Returns all control paths (rootPaths) within given topPath.
        QStringList controlPaths(const QString& topPath) const;

        // Returns (only) form paths.
        // Returned paths are rootPaths.
        QStringList formPaths() const;

        // Returns all control paths in form scope.
        // Returned paths are rootPaths.
        QStringList formScopePaths() const;

        // Returns biggest number from integer named dirs.
        // If no integer named dir exists, 0 returned.
        // If no dir exists or dirs are smaller than zero, 0 returned.
        int biggestDir(const QString& basePath) const;

        // Recalculates all uids belongs to given control and its children (all).
        // Both database and in-memory data are updated.
        void recalculateUids(Control* control) const;

        // Refactor control's id if it's already exists in db
        // If suid empty, project root searched
        void refactorId(Control* control, const QString& suid, const QString& topPath = QString()) const;

        // Update all matching 'from's to 'to's within given file
        void updateFile(const QString& filePath, const QString& from, const QString& to) const;

        // Returns true if given root path belongs to a form
        // Searches within current project's path
        bool isForm(const QString& rootPath) const;

        // Returns root path if given uid belongs to a control
        // Searches within given rootPath (root control included)
        // Searches in current project directory if given rootPath is empty
        // All controls in the given rootPath are included to search
        QString findByUid(const QString& uid, const QString& rootPath/* = QString()*/) const;

        // Returns root path if given id belongs to a control
        // Searches within given rootPath (root control included)
        // Searches in current project directory if given rootPath is empty
        // Only suid scope controls in the given rootPath are returned
        QString findById(const QString& suid, const QString& id, const QString& rootPath = QString()) const;

        // Returns true if given path is inside of owdb
        bool isInOwdb(const QString& path) const;

        // Returns root path (of parent) if given control has a parent control
        QString parentDir(const Control* control) const;

        // Build qml object form url
        QObject* requestItem(const QString& path, QQmlEngine* engine, QQmlContext* context) const;

        // Returns true if the given object is an instance of QQuickItem
        Type type(QObject* object) const;

    public:
        SaveManager* parent = nullptr;
        ParserController parserController;
};

SaveManagerPrivate::SaveManagerPrivate(SaveManager* parent)
    : QObject(parent)
    , parent(parent)
{
}

void SaveManagerPrivate::setProperty(QByteArray& propertyData, const QString& property, const QJsonValue& value) const
{
    if (propertyData.isEmpty())
        return;

    auto jobj = QJsonDocument::fromJson(propertyData).object();
    jobj[property] = value;
    propertyData = QJsonDocument(jobj).toJson();
}

QJsonValue SaveManagerPrivate::property(const QByteArray& propertyData, const QString& property) const
{
    if (propertyData.isEmpty())
        return QJsonValue();

    auto jobj = QJsonDocument::fromJson(propertyData).object();
    return jobj[property];
}

void SaveManagerPrivate::flushId(const Control* control, const QString& id) const
{
    auto propertyPath = control->dir() + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_ID, id);
    wrfile(propertyPath, propertyData);
}

void SaveManagerPrivate::flushSuid(const Control* control, const QString& suid) const
{
    auto propertyPath = control->dir() + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_SUID, suid);
    wrfile(propertyPath, propertyData);
}

bool SaveManagerPrivate::isOwctrl(const QByteArray& propertyData) const
{
    auto sign = property(propertyData, TAG_OWCTRL_SIGN).toString();
    auto uid = property(propertyData, TAG_OWCTRL_SIGN).toString();
    return (sign == SIGN_OWCTRL && !uid.isEmpty());
}

bool SaveManagerPrivate::existsInForms(const Control* control) const
{
    for (auto path : formPaths()) {
        auto propertyData = rdfile(path + separator() + DIR_THIS + separator() + FILE_PROPERTIES);
        auto id = property(propertyData, TAG_ID).toString();

        if (id == control->id())
            return true;
    }

    return false;
}

bool SaveManagerPrivate::existsInFormScope(const Control* control) const
{
    Q_ASSERT(control->form());
    for (auto path : formScopePaths()) {
        auto propertyData = rdfile(path + separator() + DIR_THIS + separator() + FILE_PROPERTIES);
        auto id = property(propertyData, TAG_ID).toString();

        if (id == control->id())
            return true;
    }

    return false;
}

bool SaveManagerPrivate::existsInParentScope(const Control* control, const QString& suid, const QString topPath) const
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

        for (auto path : parent->childrenPaths(parentRootPath)) {
            auto propertyData = rdfile(path + separator() + DIR_THIS +
                                       separator() + FILE_PROPERTIES);
            auto id = property(propertyData, TAG_ID).toString();

            if (id == control->id())
                return true;
        }

        return false;
    } else {
        QStringList paths(parentRootPath + separator() + DIR_THIS);
        paths << parent->childrenPaths(parentRootPath);

        for (auto path : paths) {
            auto propertyData = rdfile(path + separator() + DIR_THIS +
                                       separator() + FILE_PROPERTIES);
            auto id = property(propertyData, TAG_ID).toString();

            if (id == control->id())
                return true;
        }

        return false;
    }
}

QStringList SaveManagerPrivate::controlPaths(const QString& topPath) const
{
    QStringList paths;

    if (topPath.isEmpty())
        return paths;

    for (auto path : fps(FILE_PROPERTIES, topPath)) {
        auto propertyData = rdfile(path);
        if (isOwctrl(propertyData))
            paths << dname(dname(path));
    }

    return paths;
}

QStringList SaveManagerPrivate::formPaths() const
{
    QStringList paths;
    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

    if (projectDir.isEmpty())
        return paths;

    auto baseDir = projectDir + separator() + DIR_OWDB;

    for (auto dir : lsdir(baseDir)) {
        auto propertyPath = baseDir + separator() + dir + separator() +
                            DIR_THIS + separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);

        if (isOwctrl(propertyData))
            paths << dname(dname(propertyPath));
    }

    return paths;
}

QStringList SaveManagerPrivate::formScopePaths() const
{
    const QStringList fpaths = formPaths();
    QStringList paths(fpaths);

    for (auto path : fpaths)
        paths << parent->childrenPaths(dname(path));

    return paths;
}

int SaveManagerPrivate::biggestDir(const QString& basePath) const
{
    int num = 0;
    for (auto dir : lsdir(basePath))
        if (dir.toInt() > num)
            num = dir.toInt();
    return num;
}

void SaveManagerPrivate::recalculateUids(Control* control) const
{
    if (control->dir().isEmpty())
        return;

    QStringList paths, properties;

    properties << fps(FILE_PROPERTIES, control->dir());
    paths << properties;

    for (auto pfile : properties) {
        auto propertyData = rdfile(pfile);

        if (!isOwctrl(propertyData))
            continue;

        auto uid = property(propertyData, TAG_UID).toString();
        auto newUid = Control::generateUid();

        for (auto file : paths)
            updateFile(file, uid, newUid);
    }

    Control::updateUids(); // FIXME: Change with childControls()->updateUid();
}

void SaveManagerPrivate::refactorId(Control* control, const QString& suid, const QString& topPath) const
{
    if (control->id().isEmpty())
        control->setId("control");

    const auto id = control->id();

    for (int i = 1; parent->exists(control, suid, topPath); i++)
        control->setId(id + QString::number(i));
}

void SaveManagerPrivate::updateFile(const QString& filePath, const QString& from, const QString& to) const
{
    auto data = rdfile(filePath);
    data.replace(from.toUtf8(), to.toUtf8());
    wrfile(filePath, data);
}

bool SaveManagerPrivate::isForm(const QString& rootPath) const
{
    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
    auto baseDir = projectDir + separator() + DIR_OWDB;
    return (baseDir == dname(rootPath));
}

QString SaveManagerPrivate::findByUid(const QString& uid, const QString& rootPath) const
{
    QString baseDir;
    if (rootPath.isEmpty()) {
        auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

        if (projectDir.isEmpty())
            return QString();

        baseDir = projectDir;
    } else {
        baseDir = rootPath;
    }

    for (auto path : fps(FILE_PROPERTIES, baseDir)) {
        auto propertyData = rdfile(path);

        if (!isOwctrl(propertyData))
            continue;

        auto _uid = property(propertyData, TAG_UID).toString();

        if (_uid == uid)
            return dname(dname(path));
    }

    return QString();
}

QString SaveManagerPrivate::findById(const QString& suid, const QString& id, const QString& rootPath) const
{
    QString baseDir;
    if (rootPath.isEmpty()) {
        auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

        if (projectDir.isEmpty())
            return QString();

        baseDir = projectDir;
    } else {
        baseDir = rootPath;
    }

    for (auto path : fps(FILE_PROPERTIES, baseDir)) {
        auto propertyData = rdfile(path);

        if (!isOwctrl(propertyData) ||
            (parent->uid(dname(dname(path))) != suid &&
             parent->suid(dname(dname(path))) != suid))
            continue;

        auto _id = property(propertyData, TAG_ID).toString();

        if (_id == id)
            return dname(dname(path));
    }

    return QString();
}

bool SaveManagerPrivate::isInOwdb(const QString& path) const
{
    auto projectDirectory = ProjectManager::projectDirectory(ProjectManager::currentProject());

    Q_ASSERT(!projectDirectory.isEmpty());

    QString owdbPath = projectDirectory + separator() + DIR_OWDB;
    return path.contains(owdbPath, Qt::CaseInsensitive);
}

QString SaveManagerPrivate::parentDir(const Control* control) const
{
    if (control->form() ||
        control->dir().isEmpty() ||
        !parent->isOwctrl(control->dir()))
        return QString();

    return dname(dname(control->dir()));
}

QObject* SaveManagerPrivate::requestItem(const QString& path, QQmlEngine* engine, QQmlContext* context) const
{
    QQmlComponent comp(engine, QUrl(path + separator() + DIR_THIS + separator() + "main.qml"));
    auto item = comp.create(context);
    qApp->processEvents(QEventLoop::AllEvents, 20);
    return item;
}

SaveManagerPrivate::Type SaveManagerPrivate::type(QObject* object) const
{
    if (qobject_cast<QQuickItem*>(object) != nullptr)
        return Type::Quick;
    if (object->isWindowType())
        return Type::Window;
    return Type::NonGui;
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
    connect(&_d->parserController, SIGNAL(runningChanged(bool)), SIGNAL(parserRunningChanged(bool)));
}

SaveManager* SaveManager::instance()
{
    return _d ? _d->parent : nullptr;
}

bool SaveManager::initProject(const QString& projectDirectory)
{
    if (projectDirectory.isEmpty() ||
        !::exists(projectDirectory) ||
        ::exists(projectDirectory + separator() + DIR_OWDB) ||
        !cp(DIR_QRC_OWDB, projectDirectory, false, true))
        return false;

    auto propertyPath = projectDirectory + separator() + DIR_OWDB +
                        separator() + DIR_MAINFORM + separator() +
                        DIR_THIS + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    _d->setProperty(propertyData, TAG_UID, Control::generateUid());

    return wrfile(propertyPath, propertyData);
}

bool SaveManager::execProject()
{
    auto dir = ProjectManager::projectDirectory(ProjectManager::currentProject());

    if(dir.isEmpty())
        return false;

    QList<QObject*> forms;
    QMap<QString, QQmlContext*> contexes;
    auto engine = new QQmlEngine(_d->parent);
    engine->rootContext()->setContextProperty("dpi", Fit::ratio());

    for (auto formPath : formPaths()) {
        auto _masterPaths = masterPaths(formPath);
        QMap<QString, QObject*> masterResults;

        QObject* form = nullptr;
        for (auto path : _masterPaths) {
            auto masterContext = new QQmlContext(engine, engine);
            QMap<QString, QObject*> results;
            auto _childrenPaths = childrenPaths(path);
            for (auto childPath : _childrenPaths) {
                int index = _masterPaths.indexOf(childPath);
                if (index >= 0) {
                    results[childPath] = masterResults[childPath];
                } else {
                    results[childPath] = _d->requestItem(childPath, engine, masterContext); //Async
                    if (results[childPath] == nullptr)
                        return false;
                }
                masterContext->setContextProperty(id(childPath), results[childPath]);
            }

            if (_masterPaths.last() == path) {
                masterResults[path] = _d->requestItem(path, engine, masterContext); //Async
                if (masterResults[path] == nullptr)
                    return false;
                form = masterResults[path];
                forms << form;
                contexes[path] = masterContext;
            } else {
                masterResults[path] = _d->requestItem(path, engine, masterContext); //Async
                if (masterResults[path] == nullptr)
                    return false;
            }
            masterContext->setContextProperty(id(path), masterResults[path]);

            QObject* parentObject;
            for (auto result : results.keys()) {
                if (dname(dname(result)) == path)
                    parentObject = masterResults[path];

                if (_d->type(results[result]) == SaveManagerPrivate::Type::Window ||
                    _d->type(parentObject) == SaveManagerPrivate::Type::NonGui)
                    return false;
                if (_d->type(results[result]) == SaveManagerPrivate::Type::NonGui)
                    continue;

                // All children are quick (not nongui/window)
                // All forms are quick or window (not nongui)

                if (parentObject->isWindowType())
                    qobject_cast<QQuickItem*>(results[result])->setParentItem(qobject_cast<QQuickWindow*>(parentObject)->contentItem());
                else
                    qobject_cast<QQuickItem*>(results[result])->setParentItem(qobject_cast<QQuickItem*>(parentObject));

                parentObject = results[result];
            }

            for (auto result : results.keys()) {
                if (_d->type(results[result]) != SaveManagerPrivate::Type::NonGui)
                    continue;

                if (form->isWindowType())
                    qobject_cast<QQuickItem*>(results[result])->setParentItem(qobject_cast<QQuickWindow*>(form)->contentItem());
                else
                    qobject_cast<QQuickItem*>(results[result])->setParentItem(qobject_cast<QQuickItem*>(form));
            }
        }
    }

    for (auto path : contexes.keys())
        for (int i = 0; i < contexes.keys().size(); i++)
            contexes[path]->setContextProperty(id(contexes.keys().at(i)), forms.at(i));

    return true;
}

QString SaveManager::basePath()
{
    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

    if (projectDir.isEmpty())
        return QString();

    return (projectDir + separator() + DIR_OWDB);
}

QStringList SaveManager::formPaths()
{
    return _d->formPaths();
}

// Returns all children paths (rootPath) within given root path.
// Returns children only if they have match between their and given suid.
// If given suid is empty then rootPath's uid is taken.
QStringList SaveManager::childrenPaths(const QString& rootPath, QString suid)
{
    QStringList paths;

    if (rootPath.isEmpty())
        return paths;

    if (suid.isEmpty()) {
        auto propertyPath = rootPath + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        suid = _d->property(propertyData, TAG_UID).toString();
    }

    auto childrenPath = rootPath + separator() + DIR_CHILDREN;
    for (auto dir : lsdir(childrenPath)) {
        auto propertyPath = childrenPath + separator() + dir + separator() +
                            DIR_THIS + separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);

        if (_d->isOwctrl(propertyData) && _d->property(propertyData, TAG_SUID).toString() == suid) {
            paths << dname(dname(propertyPath));
            paths << childrenPaths(dname(dname(propertyPath)), suid);
        }
    }
    return paths;
}

QStringList SaveManager::masterPaths(const QString& topPath)
{
    QStringList paths;
    auto controlPaths = _d->controlPaths(topPath);

    QStringList foundSuids;
    for (auto path : controlPaths) {
        auto _suid = suid(path);
        if (!_suid.isEmpty() && !foundSuids.contains(_suid))
            foundSuids << _suid;
    }

    for (auto path : controlPaths) {
        if (foundSuids.contains(uid(path)))
            paths << path;
    }

    std::sort(paths.begin(), paths.end(),
              [](const QString& a, const QString& b)
    { return a.size() > b.size(); });

    if (paths.isEmpty() && _d->isForm(topPath))
        paths << topPath;

    return paths;
}

void SaveManager::exposeProject()
{
    auto fpaths = _d->formPaths();

    Control* lastControl;

    for (auto path : fpaths) {

        auto form = new Form(path + separator() + DIR_THIS + separator() + "main.qml");
        if (fname(path) == DIR_MAINFORM)
            form->setMain(true);
        DesignManager::formScene()->addForm(form);
        connect(form, &Form::initialized, [=] {
            form->controlTransaction()->setTransactionsEnabled(true);
        });

        lastControl = form;

        Control* parentControl;
        for (auto child : childrenPaths(path)) {
            if (dname(dname(child)) == path)
                parentControl = form;

            auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
            control->setParentItem(parentControl);
            control->refresh();
            connect(control, &Control::initialized, [=] {
                control->controlTransaction()->setTransactionsEnabled(true);
            });

            parentControl = control;
            lastControl = control;
        }
    }

    Delayer::delay(lastControl, &Control::init, true);

    emit instance()->projectExposed();
}

Control* SaveManager::exposeControl(const QString& rootPath, QString suid)
{
    auto control = new Control(rootPath + separator() + DIR_THIS +
                               separator() + "main.qml");

    Control* parentControl;
    for (auto child : childrenPaths(rootPath, suid)) {
        if (dname(dname(child)) == rootPath)
            parentControl = control;

        auto control = new Control(child + separator() + DIR_THIS + separator() + "main.qml");
        control->setParentItem(parentControl);
        control->refresh();
        connect(control, &Control::initialized, [=] {
            control->controlTransaction()->setTransactionsEnabled(true);
        });

        parentControl = control;
    }

    return control;
}

bool SaveManager::isOwctrl(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->isOwctrl(propertyData);
}

Skin SaveManager::skin(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return Skin(_d->property(propertyData, TAG_SKIN).toInt());
}

QString SaveManager::id(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_ID).toString();
}

QString SaveManager::uid(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_UID).toString();
}

QString SaveManager::suid(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_SUID).toString();
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

        if (!_d->isOwctrl(propertyData))
            continue;

        auto uid = _d->property(propertyData, TAG_UID).toString();
        auto newUid = Control::generateUid();

        for (auto file : paths)
            _d->updateFile(file, uid, newUid);
    }
}

QString SaveManager::toolCategory(const QString& toolRootPath)
{
    if (toolRootPath.isEmpty())
        return QString();

    auto propertyPath = toolRootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_CATEGORY).toString();
}

// You have to provide an valid suid, except if control is a form
// If topPath is empty, then top level project directory searched
// So, suid and topPath have to be in a valid logical relationship.
bool SaveManager::exists(const Control* control, const QString& suid, const QString& topPath)
{
    return control->form() ? _d->existsInFormScope(control) :
                             _d->existsInParentScope(control, suid, topPath);
}

bool SaveManager::addForm(Form* form)
{
    if (form->url().isEmpty())
        return false;

    if (!isOwctrl(form->dir()))
        return false;

    _d->refactorId(form, QString());

    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

    if (projectDir.isEmpty())
        return false;

    auto baseDir = projectDir + separator() + DIR_OWDB;
    auto formDir = baseDir + separator() + QString::number(_d->biggestDir(baseDir) + 1);

    if (!mkdir(formDir))
        return false;

    if (!cp(form->dir(), formDir, true))
        return false;

    form->setUrl(formDir + separator() + DIR_THIS + separator() + "main.qml");

    _d->flushId(form, form->id());
    _d->recalculateUids(form);

    emit _d->parent->databaseChanged();

    return true;
}

void SaveManager::removeForm(const Form* form)
{
    if (form->id().isEmpty() || form->url().isEmpty())
        return;

    if (form->main() || !isOwctrl(form->dir()) || !exists(form, QString()))
        return;

    rm(form->dir());

    emit _d->parent->databaseChanged();
}

bool SaveManager::addControl(Control* control, const Control* parentControl, const QString& suid, const QString& topPath)
{
    if (control->url().isEmpty())
        return false;

    if (parentControl->dir().isEmpty())
        return false;

    if (!isOwctrl(control->dir()) || !isOwctrl(parentControl->dir()))
        return false;

    _d->refactorId(control, suid, topPath);
    for (auto child : control->childControls())
        _d->refactorId(child, suid, topPath);

    auto baseDir = parentControl->dir() + separator() + DIR_CHILDREN;
    auto controlDir = baseDir + separator() + QString::number(_d->biggestDir(baseDir) + 1);

    if (!mkdir(controlDir))
        return false;

    if (!cp(control->dir(), controlDir, true))
        return false;

    control->setUrl(controlDir + separator() + DIR_THIS + separator() + "main.qml");

    _d->flushId(control, control->id());
    for (auto child : control->childControls())
        _d->flushId(child, child->id());

    _d->flushSuid(control, suid);
    _d->recalculateUids(control);

    if (_d->isInOwdb(control->dir()))
        emit _d->parent->databaseChanged();

    return true;
}

// You can only move controls within current suid scope of related control
bool SaveManager::moveControl(Control* control, const Control* parentControl)
{ //FIXME: Some controls are disappearing after moving
    if (_d->parentDir(control) == parentControl->dir())
        return true;

    if (control->id().isEmpty() || control->url().isEmpty())
        return false;

    if (parentControl->url().isEmpty())
        return false;

    if (!isOwctrl(control->dir()) || !isOwctrl(parentControl->dir()))
        return false;

    if (!parentControl->form() && suid(control->dir()) != suid(parentControl->dir()))
        return false;

    auto baseDir = parentControl->dir() + separator() + DIR_CHILDREN;
    auto controlDir = baseDir + separator() + QString::number(_d->biggestDir(baseDir) + 1);

    if (!mkdir(controlDir))
        return false;

    if (!cp(control->dir(), controlDir, true))
        return false;

    if (!rm(control->dir()))
        return false;

    control->setUrl(controlDir + separator() + DIR_THIS + separator() + "main.qml");

    if (_d->isInOwdb(control->dir()))
        emit _d->parent->databaseChanged();

    return true;
}

void SaveManager::removeControl(const Control* control)
{
    if (control->id().isEmpty() || control->url().isEmpty())
        return;

    if (!isOwctrl(control->dir()))
        return;

    rm(control->dir());

    if (_d->isInOwdb(control->dir()))
        emit _d->parent->databaseChanged();
}

void SaveManager::removeChildControlsOnly(const Control* control)
{
    if (control->id().isEmpty() || control->url().isEmpty())
        return;

    if (!isOwctrl(control->dir()))
        return;

    rm(control->dir() + separator() + DIR_CHILDREN);

    if (_d->isInOwdb(control->dir()))
        emit _d->parent->databaseChanged();
}

// You can not set id property of a top control if it's not exist in the project database
// If you want to set id property of a control that is not exist in the project database,
// then you have to provide a valid topPath
// If topPath is empty, then top level project directory searched
// So, suid and topPath have to be in a valid logical relationship.
// topPath is only necessary if property is an "id" set.
void SaveManager::setProperty(Control* control, const QString& property, const QVariant& value, const QString& topPath)
{
    if (control->dir().isEmpty() || !isOwctrl(control->dir()))
        return;

    if (property == TAG_ID) {
        if (control->id() == value.toString())
            return;

        auto _suid = suid(control->dir());
        control->setId(value.toString());
        _d->refactorId(control, _suid, topPath);

        auto propertyPath = control->dir() + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        _d->setProperty(propertyData, TAG_ID, QJsonValue(control->id()));
        wrfile(propertyPath, propertyData);
    } if (property == TAG_SKIN) {
        if (control->dir().isEmpty() || !control->form())
            return;

        auto propertyPath = control->dir() + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        _d->setProperty(propertyData, TAG_SKIN, value.toInt());
        wrfile(propertyPath, propertyData);
    } else {
        auto fileName = control->dir() + separator() + DIR_THIS +
                        separator() + "main.qml";
        ParserController::setVariantProperty(fileName, property, value);
    }

    if (_d->isInOwdb(control->dir()))
        emit _d->parent->databaseChanged();
}

void SaveManager::removeProperty(const Control* control, const QString& property)
{
    if (control->dir().isEmpty() || !isOwctrl(control->dir()) || property == TAG_ID)
        return;

    auto fileName = control->dir() + separator() + DIR_THIS +
                    separator() + "main.qml";
    ParserController::removeVariantProperty(fileName, property);

    if (_d->isInOwdb(control->dir()))
        emit _d->parent->databaseChanged();
}

QString SaveManager::pathOfId(const QString& suid, const QString& id, const QString& rootPath)
{
    return _d->findById(suid, id, rootPath);
}

bool SaveManager::parserWorking()
{
    return _d->parserController.running();
}

#include "savemanager.moc"
