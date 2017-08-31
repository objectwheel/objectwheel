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
#include <designmanager.h>

#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QTimer>
#include <QDebug>

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
        SaveManagerPrivate(SaveManager* parent);
        void setProperty(QByteArray& propertyData, const QString& property, const QJsonValue& value) const;
        QJsonValue property(const QByteArray& propertyData, const QString& property) const;
        using QObject::event;
        QJsonValue event(const QByteArray& eventsData, const QString& sign) const;
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
        bool existsInFormScope(const Control* control) const; // Searches by id.

        // Searches by id.
        // Searches control within given suid
        // If given suid is is empty, then returns existsInFormScope().
        bool existsInParentScope(const Control* control, const QString& suid) const;

        // Returns all children paths (DIR_THIS) within given root path.
        // Returns children only if they have match between their and given suid.
        // If given suid is empty then rootPath's uid is taken.
        QStringList childrenPaths(const QString& rootPath, QString suid = QString()) const;

        // Returns (only) form paths.
        // Returned paths are paths of main.qml files (DIR_THIS).
        QStringList formPaths() const;

        // Returns all control paths in form scope.
        // Returned paths are paths of main.qml files (DIR_THIS).
        QStringList formScopePaths() const;

        // Returns biggest number from integer named dirs.
        // If no integer named dir exists, 0 returned.
        // If no dir exists or dirs are smaller than zero, 0 returned.
        int biggestDir(const QString& basePath) const;

        // Recalculates all uids belongs to given control and its children (all).
        // All events/bindings/suids are updated too.
        // Both database and in-memory data are updated.
        void recalculateUids(Control* control) const;

        // Update all matching 'from's to 'to's within given file
        void updateFile(const QString& filePath, const QString& from, const QString& to) const;

        // Returns true if given root path belongs to a form
        // Searches within current project's path
        bool isForm(const QString& rootPath) const;

        // Returns root path if given uid belongs to a control
        // Searches within given rootPath (root control included)
        // Searches in current project directory if given rootPath is empty
        // All controls in the given rootPath are included to search
        QString findByUid(const QString& uid, const QString& rootPath = QString()) const;

        // Returns root path if given id belongs to a control
        // Searches within given rootPath (root control included)
        // Searches in current project directory if given rootPath is empty
        // Only suid scope controls in the given rootPath are returned
        QString findById(const QString& suid, const QString& id, const QString& rootPath = QString()) const;

        // Searches inside the suid scope
        // Returns the rootPath of a control that has the given event sign
        QString findByEventSign(const QString& suid, const QString& sign, const QString& rootPath = QString()) const;

        // Returns root path (of parent) if given control has a parent control
        QString parentDir(const Control* control) const;

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

QJsonValue SaveManagerPrivate::event(const QByteArray& eventsData, const QString& sign) const
{
    if (eventsData.isEmpty())
        return QJsonValue();

    auto jobj = QJsonDocument::fromJson(eventsData).object();
    return jobj[sign];
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
        auto propertyData = rdfile(path + separator() + FILE_PROPERTIES);
        auto id = property(propertyData, TAG_ID).toString();

        if (id == control->id())
            return true;
    }

    return false;
}

bool SaveManagerPrivate::existsInFormScope(const Control* control) const
{
    for (auto path : formScopePaths()) {
        auto propertyData = rdfile(path + separator() + FILE_PROPERTIES);
        auto id = property(propertyData, TAG_ID).toString();

        if (id == control->id())
            return true;
    }

    return false;
}

bool SaveManagerPrivate::existsInParentScope(const Control* control, const QString& suid) const
{
    if (!suid.isEmpty()) {
        auto parentRootPath = findByUid(suid);
        if (parentRootPath.isEmpty())
            return false;
        if (isForm(parentRootPath)) {
            if (existsInForms(control))
                return true;

            for (auto path : childrenPaths(parentRootPath)) {
                auto propertyData = rdfile(path + separator() + FILE_PROPERTIES);
                auto id = property(propertyData, TAG_ID).toString();

                if (id == control->id())
                    return true;
            }

            return false;
        } else {
            QStringList paths(parentRootPath + separator() + DIR_THIS);
            paths << childrenPaths(parentRootPath);

            for (auto path : paths) {
                auto propertyData = rdfile(path + separator() + FILE_PROPERTIES);
                auto id = property(propertyData, TAG_ID).toString();

                if (id == control->id())
                    return true;
            }

            return false;
        }
    } else {
        return existsInFormScope(control);
    }
}

QStringList SaveManagerPrivate::childrenPaths(const QString& rootPath, QString suid) const
{
    QStringList paths;

    if (rootPath.isEmpty())
        return paths;

    if (suid.isEmpty()) {
        auto propertyPath = rootPath + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        suid = property(propertyData, TAG_UID).toString();
    }

    auto childrenPath = rootPath + separator() + DIR_CHILDREN;
    for (auto dir : lsdir(childrenPath)) {
        auto propertyPath = childrenPath + separator() + dir + separator() +
                            DIR_THIS + separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);

        if (isOwctrl(propertyData) && property(propertyData, TAG_SUID).toString() == suid) {
            paths << dname(propertyPath);
            paths << childrenPaths(dname(dname(propertyPath)), suid);
        }
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
            paths << dname(propertyPath);
    }

    return paths;
}

QStringList SaveManagerPrivate::formScopePaths() const
{
    const QStringList fpaths = formPaths();
    QStringList paths(fpaths);

    for (auto path : fpaths)
        paths << childrenPaths(dname(path));

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
    paths << fps(FILE_EVENTS, control->dir());
    //TODO: Bindings

    for (auto pfile : properties) {
        auto propertyData = rdfile(pfile);

        if (!isOwctrl(propertyData))
            continue;

        auto uid = property(propertyData, TAG_UID).toString();
        auto newUid = Control::generateUid();

        for (auto file : paths)
            updateFile(file, uid, newUid);
    }

    Control::updateUids();
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

QString SaveManagerPrivate::findByEventSign(const QString& suid, const QString& sign, const QString& rootPath) const
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

    for (auto path : fps(FILE_EVENTS, baseDir)) {
        auto eventsData = rdfile(path);
        auto propertyData = rdfile(dname(path) + separator() + FILE_PROPERTIES);

        if (!isOwctrl(propertyData) ||
            (parent->uid(dname(dname(path))) != suid &&
             parent->suid(dname(dname(path))) != suid))
            continue;

        auto _event = event(eventsData, sign);

        if (!_event.isUndefined())
            return dname(dname(path));
    }

    return QString();
}

QString SaveManagerPrivate::parentDir(const Control* control) const
{
    if (control->form() ||
        control->dir().isEmpty() ||
        !parent->isOwctrl(control->dir()))
        return QString();

    return dname(dname(control->dir()));
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

}

QString SaveManager::basePath()
{
    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());

    if (projectDir.isEmpty())
        return QString();

    return (projectDir + separator() + DIR_OWDB);
}

void SaveManager::exposeProject()
{
    auto fpaths = _d->formPaths();

    for (auto path : fpaths) {

        auto form = new Form(path + separator() + "main.qml");
        DesignManager::formScene()->addForm(form);
        connect(form, &Form::initialized, [=] {
            form->controlTransaction()->setTransactionsEnabled(true);
        });

        Control* parentControl;
        for (auto child : _d->childrenPaths(dname(path))) {
            if ((dname(dname(dname(child))) + separator() + DIR_THIS) == path)
                parentControl = form;

            auto control = new Control(child + separator() + "main.qml");
            control->setParentItem(parentControl);
            control->refresh();
            connect(control, &Control::initialized, [=] {
                control->controlTransaction()->setTransactionsEnabled(true);
            });

            parentControl = control;
        }
    }
}

bool SaveManager::isOwctrl(const QString& rootPath)
{
    auto propertyPath = rootPath + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    return _d->isOwctrl(propertyData);
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

bool SaveManager::exists(const Control* control, const QString& suid)
{
    return control->form() ? _d->existsInFormScope(control) : _d->existsInParentScope(control, suid);
}

bool SaveManager::addForm(Form* form)
{
    if (form->id().isEmpty() || form->url().isEmpty())
        return false;

    if (!isOwctrl(form->dir()))
        return false;

    if (exists(form))
        return false;

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

bool SaveManager::addControl(Control* control, const Control* parentControl, const QString& suid)
{
    if (control->id().isEmpty() || control->url().isEmpty())
        return false;

    if (parentControl->dir().isEmpty())
        return false;

    if (!isOwctrl(control->dir()) || !isOwctrl(parentControl->dir()))
        return false;

    if (exists(control, suid))
        return false;

    auto baseDir = parentControl->dir() + separator() + DIR_CHILDREN;
    auto controlDir = baseDir + separator() + QString::number(_d->biggestDir(baseDir) + 1);

    if (!mkdir(controlDir))
        return false;

    if (!cp(control->dir(), controlDir, true))
        return false;

    control->setUrl(controlDir + separator() + DIR_THIS + separator() + "main.qml");

    _d->flushId(control, control->id());
    _d->flushSuid(control, suid);
    _d->recalculateUids(control);

    emit _d->parent->databaseChanged();

    return true;
}

bool SaveManager::moveControl(Control* control, const Control* parentControl)
{
    if (_d->parentDir(control) == parentControl->dir())
        return true;

    if (control->id().isEmpty() || control->url().isEmpty())
        return false;

    if (parentControl->dir().isEmpty())
        return false;

    if (!isOwctrl(control->dir()) || !isOwctrl(parentControl->dir()))
        return false;

    if (suid(control->dir()) != suid(parentControl->dir()))
        return false;

    if (!exists(control, suid(control->dir())) ||
        !exists(parentControl, suid(parentControl->dir())))
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

    emit _d->parent->databaseChanged();

    return true;
}

void SaveManager::setProperty(const Control* control, const QString& property, const QVariant& value)
{
    if (control->dir().isEmpty() || !isOwctrl(control->dir()))
        return;

    if (property == TAG_ID) {
        auto _suid = suid(control->dir());
        Q_ASSERT(_suid.isEmpty() || !exists(control, _suid));

        auto propertyPath = control->dir() + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        _d->setProperty(propertyData, TAG_ID, QJsonValue::fromVariant(value));
        wrfile(propertyPath, propertyData);
    } else {
        auto fileName = control->dir() + separator() + DIR_THIS +
                        separator() + "main.qml";
        ParserController::setVariantProperty(fileName, property, value);
    }
}

void SaveManager::removeProperty(const Control* control, const QString& property)
{
    if (control->dir().isEmpty() || !isOwctrl(control->dir()) || property == TAG_ID)
        return;

    auto fileName = control->dir() + separator() + DIR_THIS +
                    separator() + "main.qml";
    ParserController::removeVariantProperty(fileName, property);
}

void SaveManager::setEvent(const Control* control, const SaveManager::Event& event)
{
    setEvent(control->dir(), event);
}

void SaveManager::setEvent(const QString& rootPath, const SaveManager::Event& event)
{
    if (!_d->findByEventSign(suid(rootPath), event.sign).isEmpty())
        return;

    if (rootPath.isEmpty() || !isOwctrl(rootPath))
        return;

    auto eventsPath = rootPath + separator() + DIR_THIS +
                      separator() + FILE_EVENTS;
    auto eventsData = rdfile(eventsPath);

    QJsonObject jobj;
    jobj[TAG_EVENT_NAME] = event.name;
    jobj[TAG_EVENT_CODE] = QString(QByteArray().insert(0, event.code).toBase64());
    QJsonObject job1 = QJsonDocument::fromJson(eventsData).object();
    job1[event.sign] = jobj;
    wrfile(eventsPath, QJsonDocument(job1).toJson());
}

void SaveManager::updateEvent(const QString& suid, const QString& sign, const SaveManager::Event& event)
{
    auto rootPath = _d->findByEventSign(suid, sign);

    if (rootPath.isEmpty())
        return;

    removeEvent(suid, sign);
    setEvent(rootPath, event);
}

void SaveManager::removeEvent(const QString& suid, const QString& sign)
{
    auto rootPath = _d->findByEventSign(suid, sign);

    if (rootPath.isEmpty() || !isOwctrl(rootPath))
        return;

    auto eventsPath = rootPath + separator() + DIR_THIS +
                      separator() + FILE_EVENTS;
    auto eventsData = rdfile(eventsPath);

    QJsonObject jobj = QJsonDocument::fromJson(eventsData).object();
    jobj.remove(sign);
    wrfile(eventsPath, QJsonDocument(jobj).toJson());
}

SaveManager::Event SaveManager::event(const QString& suid, const QString& sign, const QString& rootPath)
{
    Event event;
    auto path = _d->findByEventSign(suid, sign, rootPath);

    if (path.isEmpty())
        return event;

    auto eventsPath = path + separator() + DIR_THIS +
                      separator() + FILE_EVENTS;
    auto eventsData = rdfile(eventsPath);

    auto jobj = QJsonDocument::fromJson(eventsData).object()[sign].toObject();

    if (jobj.isEmpty())
        return event;

    event.sign = sign;
    event.name = jobj[TAG_EVENT_NAME].toString();
    event.code = jobj[TAG_EVENT_CODE].toString();

    return event;
}

QString SaveManager::idOfEventOwner(const QString& suid, const QString& sign, const QString& rootPath)
{
    auto path = _d->findByEventSign(suid, sign, rootPath);

    if (path.isEmpty())
        return QString();

    auto propertyPath = path + separator() + DIR_THIS +
                        separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);

    return _d->property(propertyData, TAG_ID).toString();
}

QString SaveManager::pathOfId(const QString& suid, const QString& id, const QString& rootPath)
{
    return _d->findById(suid, id, rootPath);
}

bool SaveManager::inprogress()
{
    return _d->parserController.running();
}

#include "savemanager.moc"
