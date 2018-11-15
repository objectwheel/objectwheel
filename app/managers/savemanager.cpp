#include <savemanager.h>
#include <saveutils.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <form.h>
#include <parserutils.h>
#include <hashfactory.h>
#include <zipper.h>
#include <controlpropertymanager.h>
#include <utilityfunctions.h>

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QRegularExpression>

namespace {

bool countIdInProjectForms(const QString& id)
{
    Q_ASSERT(!id.isEmpty());

    int counter = 0;
    for (const QString& formPath : SaveUtils::formPaths(ProjectManager::dir())) {
        if (ParserUtils::id(SaveUtils::toUrl(formPath)) == id)
            ++counter;
    }

    return counter;
}

int countIdInControlChildrenScope(const QString& id, const QString rootPath)
{
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(!rootPath.isEmpty());

    int counter = 0;
    for (const QString& controlRootPath : SaveUtils::childrenPaths(rootPath)) {
        if (ParserUtils::id(SaveUtils::toUrl(controlRootPath)) == id)
            ++counter;
    }

    return counter;
}

inline int countIdInProjectFormScope(const QString& id, const QString formRootPath)
{
    return countIdInProjectForms(id) + countIdInControlChildrenScope(id, formRootPath);
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
            ::exists(SaveUtils::toOwdbDir(projectDirectory)) ||
            !Zipper::extractZip(rdfile(tr(":/templates/template%1.zip").arg(templateNumber)),
                                projectDirectory)) {
        return false;
    }

    SaveUtils::regenerateUids(SaveUtils::toOwdbDir(projectDirectory));

    return true;
}

bool SaveManager::addForm(const QString& formRootPath)
{
    Q_ASSERT(!ProjectManager::dir().isEmpty());

    if (!SaveUtils::isOwctrl(formRootPath)) {
        qWarning("SaveManager::addControl: Failed. Form data broken.");
        return false;
    }

    const QString& targetOwdbDir = SaveUtils::toOwdbDir(ProjectManager::dir());
    const QString& newFormRootPath = targetOwdbDir + separator() +
            QString::number(SaveUtils::biggestDir(targetOwdbDir) + 1);

    if (!mkdir(newFormRootPath)) {
        qWarning("SaveManager::addControl: Failed. Cannot create the new form root path.");
        return false;
    }

    if (!cp(formRootPath, newFormRootPath, true)) {
        qWarning("SaveManager::addControl: Failed. Cannot copy the control to its new root path.");
        return false;
    }

    repairIdsInProjectFormScope(newFormRootPath, formRootPath);

    SaveUtils::regenerateUids(newFormRootPath);

    return true;
}

void SaveManager::removeForm(const QString& formRootPath)
{
    if (SaveUtils::isMain(formRootPath)
            || !SaveUtils::isOwctrl(formRootPath)
            || countIdInProjectForms(SaveUtils::id(formRootPath)) == 0) {
        return;
    }

    rm(formRootPath);
}

void SaveManager::setupFormGlobalConnections(const QString& formRootPath)
{
    Q_ASSERT(SaveUtils::isOwctrl(formRootPath));

    const QString& id = ParserUtils::id(SaveUtils::toUrl(formRootPath));
    Q_ASSERT(!id.isEmpty());

    QString FormJS(id + "JS");
    FormJS.replace(0, 1, FormJS[0].toUpper());

    QByteArray content = rdfile(SaveUtils::toUrl(formRootPath));
    Q_ASSERT(!content.isEmpty());

    content.replace("// GlobalConnectionHere",
                    QString::fromUtf8("Component.onCompleted: %1.%2_onCompleted()").arg(FormJS).arg(id).toUtf8());

    wrfile(SaveUtils::toUrl(formRootPath), content);

    const QString& globalJSPath = SaveUtils::toGlobalDir(ProjectManager::dir()) + separator() + id + ".js";
    QString js = rdfile(":/resources/other/form.js");
    js = js.arg(id);
    if (!exists(globalJSPath))
        wrfile(globalJSPath, js.toUtf8());
    else
        return (void) (qWarning() << tr("SaveManager::setupFormGlobalConnections: Global %1 file is already exists.").arg(id + ".js"));

    emit instance()->formGlobalConnectionsDone(FormJS, id);
}

bool SaveManager::addControl(const QString& controlRootPath, const QString& targetParentControlRootPath, const QString& targetFormRootPath)
{
    if (!SaveUtils::isOwctrl(controlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Control data broken.");
        return false;
    }

    if (!SaveUtils::isOwctrl(targetParentControlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Parent control data broken.");
        return false;
    }

    if (!SaveUtils::isOwctrl(targetFormRootPath)) {
        qWarning("SaveManager::addControl: Failed. Target (scope resolution) form data broken.");
        return false;
    }

    const QString& targetParentControlChildrenDir = SaveUtils::toChildrenDir(targetParentControlRootPath);
    const QString& newControlRootPath = targetParentControlChildrenDir + separator() +
            QString::number(SaveUtils::biggestDir(targetParentControlChildrenDir) + 1);

    if (!mkdir(newControlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Cannot create the new control root path.");
        return false;
    }

    if (!cp(controlRootPath, newControlRootPath, true)) {
        qWarning("SaveManager::addControl: Failed. Cannot copy the control to its new root path.");
        return false;
    }

    repairIdsInProjectFormScope(newControlRootPath, targetFormRootPath);

    SaveUtils::regenerateUids(newControlRootPath);

    return true;
}

// You can only move controls within current suid scope of related control
bool SaveManager::moveControl(Control* control, const Control* parentControl)
{
    if (SaveUtils::toParentDir(control->dir()) == parentControl->dir())
        return true;

    if (control->id().isEmpty() || control->url().isEmpty())
        return false;

    if (parentControl->url().isEmpty())
        return false;

    if (!SaveUtils::isOwctrl(control->dir()) || !SaveUtils::isOwctrl(parentControl->dir()))
        return false;

    if (!parentControl->form())
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

void SaveManager::removeControl(const QString& rootPath)
{
    if (!SaveUtils::isOwctrl(rootPath))
        return;

    rm(rootPath);
}

/*!
    Empty ids corrected
    Non-unique ids made unique
    Id conflicts on main.qml and control meta files fixed (the id of the control meta file takes precedence)
*/
void SaveManager::repairIdsInProjectFormScope(const QString& rootPath, const QString& formRootPath)
{
    QStringList controlRootPaths;
    controlRootPaths.append(rootPath);
    controlRootPaths.append(SaveUtils::childrenPaths(rootPath));

    for (const QString& controlRootPath : controlRootPaths) {
        const QString& idOrig = SaveUtils::id(controlRootPath);

        QString id(idOrig);
        if (id.isEmpty())
            id = "control";

        while (countIdInProjectFormScope(id, formRootPath) > (id == idOrig ? 1 : 0))
            id = UtilityFunctions::increasedNumberedText(id, false, true);

        ParserUtils::setId(SaveUtils::toUrl(controlRootPath), id);
    }
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
//        WARNING refactorId(control, SaveUtils::suid(control->dir()), topPath, false);
        value = control->id();
        ParserUtils::setId(control->url(), value);
    } else {
        ParserUtils::setProperty(control->url(), property, value);
    }

    ProjectManager::updateLastModification(ProjectManager::hash());

    instance()->propertyChanged(control, property, value);
}