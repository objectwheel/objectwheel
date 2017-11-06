#include <savemanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <parsercontroller.h>
#include <formswidget.h>
#include <mainwindow.h>
#include <control.h>
#include <algorithm>
#include <delayer.h>
#include <executivewidget.h>
#include <parserworker.h>
#include <controlwatcher.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlProperty>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickView>

/******************************************************************/
/**          D A T A B A S E  I N F R A S T R U C T U R E        **/
/******************************************************************/
/** POPERTIES:                                                   **/
/*  Elements: Main-form(master), Ordinary-form(master),           */
/*            Child-item(master), Child-item(non-master)          */
/*  Types:    Quick, Non-gui, Window                              */
/*                                                                */
/** RULES:                                                       **/
/* - Non-gui elements can not be master (or form)                 */
/* - Main form has to be window type                              */
/* - Other forms could be quick item or window type (not non-gui) */
/*   unless main form has a Phone skin, in this case other forms  */
/*   (except main form) has to be item (not window or non-gui)    */
/* - Children could be non-gui or quick item type (not window)    */
/* - A form has to be master item                                 */
/******************************************************************/

//!
//! ******************* [SaveManagerPrivate] *******************
//!

enum Type {
    Quick,
    Window,
    NonGui
};

class SaveManagerPrivate : public QObject
{
        Q_OBJECT
        // Forms: All forms (children not included).
        // Form Scope: All forms + Primary children of them.
        // Parent Scope: Parent + Primary children of Parent.
        // None of member functions checks whether given controls' ids or dirs are valid or not.

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

        // Returns true if given path belongs to main form
        // It doesn't check whether rootPath belong to a form or not.
        bool isMain(const QString& rootPath);

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
        QObject* requestItem(ExecError& err, const QString& path, QQmlEngine* engine, QQmlContext* context) const;

        // Build qml object form data
        QObject* requestItem(ExecError& err, const QByteArray& data, const QString& path, QQmlEngine* engine, QQmlContext* context) const;

        // Returns true if the given object is an instance of QQuickItem
        Type type(QObject* object) const;

    public:
        SaveManager* parent = nullptr;
        ParserController parserController;
        ExecutiveWidget executiveWidget;

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
    auto topPath = control->dir();
    auto fromUid = parent->suid(topPath);
    if (!fromUid.isEmpty()) {
        for (auto path : fps(FILE_PROPERTIES, topPath)) {
            if (SaveManager::suid(dname(dname(path))) == fromUid) {
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
        setProperty(propertyData, TAG_SUID, suid);
        wrfile(propertyPath, propertyData);
    }
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

bool SaveManagerPrivate::isMain(const QString& rootPath)
{
    return (fname(rootPath) == DIR_MAINFORM);
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

QObject* SaveManagerPrivate::requestItem(ExecError& err,
                                         const QString& path, QQmlEngine* engine, QQmlContext* context) const
{
    QQmlComponent comp(engine, QUrl(path + separator() +
                                    DIR_THIS + separator() + "main.qml"));
    auto item = comp.create(context);
    if (!comp.errors().isEmpty()) {
        err.type = CodeError;
        err.id = parent->id(path);
        err.errors = comp.errors();
    } else {
        engine->setObjectOwnership(item, QQmlEngine::JavaScriptOwnership);
        if (type(item) == Window) {
            ((QQuickWindow*)item)->setX(parent->x(path));
            ((QQuickWindow*)item)->setY(parent->y(path));
            ((QQuickWindow*)item)->setWidth(parent->width(path));
            ((QQuickWindow*)item)->setHeight(parent->height(path));
        } else if (type(item) == Quick) {
            ((QQuickItem*)item)->setX(parent->x(path));
            ((QQuickItem*)item)->setY(parent->y(path));
            ((QQuickItem*)item)->setWidth(parent->width(path));
            ((QQuickItem*)item)->setHeight(parent->height(path));
            ((QQuickItem*)item)->setZ(parent->z(path));
        }
    }
    return item;
}

QObject* SaveManagerPrivate::requestItem(ExecError& err, const QByteArray& data,
                                         const QString& path, QQmlEngine* engine, QQmlContext* context) const
{
    QQmlComponent comp(engine);
    comp.setData(data, QUrl(path + separator() +
                            DIR_THIS + separator() + "main.qml"));
    auto item = comp.create(context);
    if (!comp.errors().isEmpty()) {
        err.type = CodeError;
        err.id = parent->id(path);
        err.errors = comp.errors();
    } else {
        engine->setObjectOwnership(item, QQmlEngine::JavaScriptOwnership);
        if (type(item) == Window) {
            ((QQuickWindow*)item)->setX(parent->x(path));
            ((QQuickWindow*)item)->setY(parent->y(path));
            ((QQuickWindow*)item)->setWidth(parent->width(path));
            ((QQuickWindow*)item)->setHeight(parent->height(path));
        } else if (type(item) == Quick) {
            ((QQuickItem*)item)->setX(parent->x(path));
            ((QQuickItem*)item)->setY(parent->y(path));
            ((QQuickItem*)item)->setWidth(parent->width(path));
            ((QQuickItem*)item)->setHeight(parent->height(path));
            ((QQuickItem*)item)->setZ(parent->z(path));
        }
    }
    return item;
}

Type SaveManagerPrivate::type(QObject* object) const
{
    if (qobject_cast<QQuickItem*>(object) != nullptr)
        return Quick;
    if (object->isWindowType())
        return Window;
    return NonGui;
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

//WARNING: Problem with scope resolution,
//         Component.onCompleted: Why items names are not reachable in this slot?
//FIXME: Change the name of default context property 'dpi' everywhere
//FIXME: Why we can't access any children of a form from another form like this: form1.btnOk.click()
//WARNING: Update error messages
ExecError SaveManager::execProject()
{
    ExecError error;
    Skin mainSkin = Skin::Invalid;

    for (auto formPath : formPaths()) {
        if (_d->isMain(formPath))
            mainSkin = skin(formPath);
    }

    if (mainSkin == Skin::Invalid) {
        error.type = CommonError;
        return error;
    }

    QList<QObject*> forms;
    QQuickWindow* mainWindow = nullptr;
    QMap<QString, QQmlContext*> formContexes;
    auto engine = new QQmlEngine(_d->parent);

    engine->rootContext()->setContextProperty("dpi", Fit::ratio());
    engine->setOutputWarningsToStandardError(false);

    // Spin for forms (top level masters)
    for (auto formPath : formPaths()) {
        auto _masterPaths = masterPaths(formPath);

        // Spin for masters inside the form (form itself included)
        QMap<QString, QObject*> masterResults;
        for (auto masterPath : _masterPaths) {
            const bool isForm = (_masterPaths.last() == masterPath);
            auto masterContext = new QQmlContext(engine, engine);

            //! Spin for child items of the master
            QMap<QString, QObject*> childResults;
            for (auto childPath : childrenPaths(masterPath)) {
                int index = _masterPaths.indexOf(childPath);
                if (index >= 0) {
                    childResults[childPath] = masterResults[childPath];
                } else {
                    childResults[childPath] = _d->requestItem(error,
                                                              childPath, engine, masterContext);
                    if (childResults[childPath] == nullptr) {
                        engine->deleteLater();
                        return error;
                    }
                    if (_d->type(childResults[childPath]) == Window) {
                        engine->deleteLater();
                        error.type = ChildIsWindowError;
                        return error;
                    }
                }
                masterContext->setContextProperty(id(childPath),
                                                  childResults[childPath]);
                qApp->processEvents(QEventLoop::AllEvents, 10);
            }

            //! Make form invisible, if it's a window type
            if (isForm && (mainSkin == Skin::PhonePortrait ||
                           mainSkin == Skin::PhoneLandscape)) { // Check If it's a form (top level master) and skin is mobile
                auto url = masterPath + separator() +
                           DIR_THIS + separator() + "main.qml";
                auto formData = rdfile(url);
                ParserWorker parserWorker;
                bool isWindow = parserWorker.typeName(formData).contains("Window");
                if (isWindow) {//If form is a window type
                    //BUG: Possible bug if property 'visible' is a binding
                    qApp->processEvents(QEventLoop::AllEvents, 10);
                    parserWorker.setVariantProperty(formData, url, "visible", false);
                }
                if (isWindow && !_d->isMain(masterPath)) {
                    engine->deleteLater();
                    error.type = MultipleWindowsForMobileError;
                    return error;
                }
                masterResults[masterPath] = _d->requestItem(error,
                  formData, masterPath, engine, masterContext);
            } else {
                masterResults[masterPath] = _d->requestItem(error,
                  masterPath, engine, masterContext);
            }

            qApp->processEvents(QEventLoop::AllEvents, 10);
            if (masterResults[masterPath] == nullptr) {
                engine->deleteLater();
                return error;
            }

            //! Catch this (current spin's) master item
            if (isForm) { // If it's a form (top level master)
                auto form = masterResults[masterPath];
                if (_d->type(form) == NonGui) {
                    engine->deleteLater();
                    error.type = FormIsNonGui;
                    return error;
                }
                if (_d->isMain(masterPath)) {
                    if (!(mainWindow = qobject_cast<QQuickWindow*>(form))) {
                        engine->deleteLater();
                        error.type = MainFormIsntWindowError;
                        return error;
                    }
                }
                forms << form;
                formContexes[masterPath] = masterContext;
            } else { // If it's a "master child inside of the form"
                auto masterItem = masterResults[masterPath];
                if (_d->type(masterItem) == Window) {
                    engine->deleteLater();
                    error.type = ChildIsWindowError;
                    return error;
                }
                if (_d->type(masterItem) == NonGui) {
                    engine->deleteLater();
                    error.type = MasterIsNonGui;
                    return error;
                }
            }
            masterContext->setContextProperty(id(masterPath),
                                              masterResults[masterPath]);

            //! Place child items into master item visually
            // Only non-master nongui children were passed (because they don't have a visual parent)
            // Others are handled in anyway, either here or above(invalid cases)
            QMap<QString, QObject*> pmap;
            pmap[masterPath] = masterResults[masterPath];
            for (auto result : childResults.keys()) {
                auto pobject = pmap.value(dname(dname(result))); // Master item (a form(master) or a child master)
                if (_d->type(childResults[result]) == NonGui) // Child item (master or non-master, but not form)
                    continue;

                // All childs are quick (not nongui/window)
                // All masters are quick or window (not nongui)

                //NOTE: What if ApplicationWindow's some properties are binding?
                if (_d->type(pobject) == Window) {
                    static_cast<QQuickItem*>(childResults[result])->setParentItem(
                      static_cast<QQuickWindow*>(pobject)->contentItem());
                } else {
                    static_cast<QQuickItem*>(childResults[result])->setParentItem(
                      static_cast<QQuickItem*>(pobject));
                }

                pmap[result] = childResults[result];
                qApp->processEvents(QEventLoop::AllEvents, 10);
            }
        }
    }

    if (mainWindow == nullptr) {
        engine->deleteLater();
        error.type = NoMainForm;
        return error;
    }

    for (auto formPath : formContexes.keys()) {
        for (int i = 0; i < formContexes.keys().size(); i++) { //Don't change 'keys().size()'
            formContexes[formPath]->setContextProperty(
              id(formContexes.keys().at(i)), forms.at(i));
        }
    }

    qApp->processEvents(QEventLoop::AllEvents, 10);

    QEventLoop loop;
    if (mainSkin == Skin::PhonePortrait ||
        mainSkin == Skin::PhoneLandscape) {
        _d->executiveWidget.setSkin(mainSkin);
        _d->executiveWidget.setWindow(mainWindow);
        _d->executiveWidget.show();
        connect(&_d->executiveWidget, SIGNAL(done()),
          &loop, SLOT(quit()));
    } else {
        connect(MainWindow::instance(),
          SIGNAL(quitting()), &loop, SLOT(quit()));
        connect(mainWindow, SIGNAL(closing(QQuickCloseEvent*)),
          &loop, SLOT(quit()));
    }
    loop.exec();

    engine->deleteLater();
    return error;
}

void SaveManager::exposeProject()
{
    auto fpaths = _d->formPaths();

    for (auto path : fpaths) {

        auto form = new Form(path + separator() +
          DIR_THIS + separator() + "main.qml");
        if (_d->isMain(path))
            form->setMain(true);
        DesignManager::formScene()->addForm(form);

        qApp->processEvents(QEventLoop::AllEvents, 10);
        QMap<QString, Control*> pmap;
        pmap[path] = form;
        for (auto child : childrenPaths(path)) {
            auto pcontrol = pmap.value(dname(dname(child)));
            auto control = new Control(child + separator() +
              DIR_THIS + separator() + "main.qml", FormGui);
            control->setParentItem(pcontrol);
            control->refresh();

            pmap[child] = control;
            qApp->processEvents(QEventLoop::AllEvents, 10);
        }
    }

    emit instance()->projectExposed();
}

Control* SaveManager::exposeControl(const QString& rootPath,
  const DesignMode& mode, QString suid)
{
    auto control = new Control(rootPath + separator() +
      DIR_THIS + separator() + "main.qml", mode);

    QMap<QString, Control*> pmap;
    pmap[rootPath] = control;
    for (auto child : childrenPaths(rootPath, suid)) {
        auto pcontrol = pmap.value(dname(dname(child)));
        auto control = new Control(child + separator() +
          DIR_THIS + separator() + "main.qml", mode);
        control->setParentItem(pcontrol);
        control->refresh();
        pmap[child] = control;
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

bool SaveManager::isMain(const QString& rootPath)
{
    return _d->isMain(rootPath);
}

Skin SaveManager::skin(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return Skin(_d->property(propertyData, TAG_SKIN).toInt());
}

qreal SaveManager::x(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_X).toDouble();
}

qreal SaveManager::y(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_Y).toDouble();
}

qreal SaveManager::z(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_Z).toDouble();
}

qreal SaveManager::width(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_WIDTH).toDouble();
}

qreal SaveManager::height(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->property(propertyData, TAG_HEIGHT).toDouble();
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

    for (auto child : control->childControls())
        child->setUrl(child->url().replace(control->dir(), controlDir));

    control->setUrl(controlDir + separator() + DIR_THIS + separator() + "main.qml");

    _d->flushId(control, control->id());
    for (auto child : control->childControls())
        _d->flushId(child, child->id());

    _d->flushSuid(control, suid);
    _d->recalculateUids(control); //for all

    if (_d->isInOwdb(control->dir()))
        emit _d->parent->databaseChanged();

    return true;
}

// You can only move controls within current suid scope of related control
bool SaveManager::moveControl(Control* control, const Control* parentControl)
{
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
void SaveManager::setProperty(Control* control, const QString& property,
  const QVariant& value, const QString& topPath)
{
    if (control->dir().isEmpty() ||
        !isOwctrl(control->dir()))
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
    } else if (property == TAG_SKIN) {
        if (!control->form())
            return;

        auto propertyPath = control->dir() + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        _d->setProperty(propertyData, TAG_SKIN, value.toInt());
        wrfile(propertyPath, propertyData);
    } else if (property == TAG_X || property == TAG_Y || property == TAG_Z ||
       property == TAG_WIDTH || property == TAG_HEIGHT) {
        auto propertyPath = control->dir() + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        _d->setProperty(propertyData, property, value.toReal());
        wrfile(propertyPath, propertyData);
    } else {
        if (control->hasErrors())
            return;
        auto fileName = control->dir() + separator() + DIR_THIS +
                        separator() + "main.qml";
        ParserController::setVariantProperty(fileName, property, value);
    }

    if (_d->isInOwdb(control->dir())) //FIXME
        emit _d->parent->databaseChanged();
}

void SaveManager::removeProperty(const Control* control, const QString& property)
{
    if (control->dir().isEmpty() ||
        control->hasErrors() ||
        !isOwctrl(control->dir()) ||
        property == TAG_ID ||
        property == TAG_SKIN)
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
