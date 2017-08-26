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
        void flushGuid(const Control* control, const QString& guid) const;
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
        // Searches control in given parent's scope
        // If given parent control is is empty, then returns existsInFormScope().
        bool existsInParentScope(const Control* control, const Control* parentControl) const; // Searches by id.

        // Returns all children paths (DIR_THIS) within given root path.
        // Returns children only if they have match between their and given guid.
        // If given guid is empty then rootPath's uid is taken.
        QStringList childrenPaths(const QString& rootPath, QString guid = QString()) const;

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
        // All events/bindings/guids are updated too.
        // Both database and in-memory data are updated.
        void recalculateUids(Control* control) const;

        // Update all matching 'from's to 'to's within given file
        void updateFile(const QString& filePath, const QString& from, const QString& to) const;

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

void SaveManagerPrivate::flushGuid(const Control* control, const QString& guid) const
{
    auto propertyPath = control->dir() + separator() + FILE_PROPERTIES;
    auto propertyData = rdfile(propertyPath);
    setProperty(propertyData, TAG_GUID, guid);
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

bool SaveManagerPrivate::existsInParentScope(const Control* control, const Control* parentControl) const
{
    if (parentControl) {
        if (parentControl->form()) {
            if (existsInForms(control))
                return true;

            for (auto path : childrenPaths(dname(parentControl->dir()))) {
                auto propertyData = rdfile(path + separator() + FILE_PROPERTIES);
                auto id = property(propertyData, TAG_ID).toString();

                if (id == control->id())
                    return true;
            }

            return false;
        } else {
            QStringList paths(parentControl->dir());
            paths << childrenPaths(dname(parentControl->dir()));

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

QStringList SaveManagerPrivate::childrenPaths(const QString& rootPath, QString guid) const
{
    QStringList paths;

    if (rootPath.isEmpty())
        return paths;

    if (guid.isEmpty()) {
        auto propertyPath = rootPath + separator() + DIR_THIS +
                            separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);
        guid = property(propertyData, TAG_UID).toString();
    }

    auto childrenPath = rootPath + separator() + DIR_CHILDREN;
    for (auto dir : lsdir(childrenPath)) {
        auto propertyPath = childrenPath + separator() + dir + separator() +
                            DIR_THIS + separator() + FILE_PROPERTIES;
        auto propertyData = rdfile(propertyPath);

        if (isOwctrl(propertyData) && property(propertyData, TAG_GUID).toString() == guid) {
            paths << dname(propertyPath);
            paths << childrenPaths(dname(dname(propertyPath)), guid);
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
    auto rootPath = dname(control->dir());

    properties << fps(FILE_PROPERTIES, rootPath);
    paths << properties;
    paths << fps(FILE_EVENTS, rootPath);
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

void SaveManager::exposeProject()
{
    auto fpaths = _d->formPaths();

    for (auto path : fpaths) {

        auto form = new Form(path + separator() + "main.qml");
        form->setDir(path);
        DesignManager::formScene()->addForm(form);

        Control* parentControl;
        for (auto child : _d->childrenPaths(dname(path))) {
            if ((dname(dname(dname(child))) + separator() + DIR_THIS) == path)
                parentControl = form;

            auto control = new Control(child + separator() + "main.qml");
            control->setDir(child);
            control->setParentItem(parentControl);
            control->refresh();

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

bool SaveManager::exists(const Control* control, const Control* parentControl)
{
    return control->form() ? _d->existsInFormScope(control) : _d->existsInParentScope(control, parentControl);
}

bool SaveManager::addForm(Form* form)
{
    if (form->id().isEmpty() || form->url().isEmpty())
        return false;

    if (!isOwctrl(dname(dname(form->url()))))
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

    if (!cp(dname(dname(form->url())), formDir, true))
        return false;

    form->setDir(formDir + separator() + DIR_THIS);

    _d->recalculateUids(form);

    emit _d->parent->databaseChanged();

    return true;
}

bool SaveManager::addControl(Control* control, const Control* parentControl, const QString& guid)
{
    if (control->id().isEmpty() || control->url().isEmpty())
        return false;

    if (parentControl->dir().isEmpty())
        return false;

    if (!isOwctrl(dname(dname(control->url()))))
        return false;

    if (exists(control, parentControl))
        return false;

    auto baseDir = dname(parentControl->dir()) + separator() + DIR_CHILDREN;
    auto controlDir = baseDir + separator() + QString::number(_d->biggestDir(baseDir) + 1);

    if (!mkdir(controlDir))
        return false;

    if (!cp(dname(dname(control->url())), controlDir, true))
        return false;

    control->setDir(controlDir + separator() + DIR_THIS);

    _d->flushGuid(control, guid);
    _d->recalculateUids(control);

    emit _d->parent->databaseChanged();

    return true;
}

// TODO: dir olayını kaldır, url'leri güncelle
// TODO: set/remove property olaylarını kodla,
// control transaction'daki yorum satırlarını kaldır
// programı basit 3 butonla test et
// TODO: expose edilen projede control'lerin pos'ları bozuk
// sebebi control transaction'ın deaktif olması mı? test et.
// TODO: Tüm toolları Owctrl'ye göre ayarla

void SaveManager::setProperty(const Control* control, const QString& property, const QVariant& value)
{
//    if (control->dir().isEmpty() ||
//        !exists(control) ||
//        !isOwctrl(dname(control->dir())))
//        return;

//    if (property == TAG_ID) {
//        auto propertyPath = control->dir() + separator() + FILE_PROPERTIES;
//        auto propertyData = rdfile(propertyPath);
//        setProperty(propertyData, TAG_GUID, guid);
//        wrfile(propertyPath, propertyData);
//    }
//    if (saveDirectory(id).isEmpty()) return;
//    auto filename = saveDirectory(id) + separator() + "main.qml";
//    ParserController::setVariantProperty(filename, property, value);
//    QmlEditor::clearCacheFor(saveDirectory(id), true);
}

void SaveManager::removeProperty(const Control* control, const QString& property)
{
//    if (saveDirectory(id).isEmpty()) return;
//    auto filename = saveDirectory(id) + separator() + "main.qml";
//    ParserController::removeVariantProperty(filename, property);
}

bool SaveManager::inprogress()
{
    return _d->parserController.running();
}

#include "savemanager.moc"
