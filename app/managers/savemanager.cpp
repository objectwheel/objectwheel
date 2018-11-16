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
//! WARNING: Bu dosyadaki warning leri ve yukarıdaki includeları düzelt

namespace {

QString detectedFormRootPath(const QString& rootPath)
{
    //! FIXME: This might crash on Windows due to back-slash path names
    Q_ASSERT(!ProjectManager::dir().isEmpty());
    const QString& owdbDir = SaveUtils::toOwdbDir(ProjectManager::dir()) + separator();
    const QString& formRootPath = QRegularExpression("^" + owdbDir + "\\d+").match(rootPath).captured();
    Q_ASSERT(!formRootPath.isEmpty());
    Q_ASSERT(exists(formRootPath));
    Q_ASSERT(SaveUtils::isForm(formRootPath));
    return formRootPath;
}

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
        qWarning("SaveManager::addForm: Failed. Form data broken.");
        return false;
    }

    const QString& targetOwdbDir = SaveUtils::toOwdbDir(ProjectManager::dir());
    const QString& newFormRootPath = targetOwdbDir + separator() +
            QString::number(SaveUtils::biggestDir(targetOwdbDir) + 1);

    if (!mkdir(newFormRootPath)) {
        qWarning("SaveManager::addForm: Failed. Cannot create the new form root path.");
        return false;
    }

    if (!cp(formRootPath, newFormRootPath, true)) {
        qWarning("SaveManager::addForm: Failed. Cannot copy the control to its new root path.");
        return false;
    }

    repairIds(newFormRootPath, true);

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

bool SaveManager::addControl(const QString& controlRootPath, const QString& targetParentControlRootPath)
{
    if (!SaveUtils::isOwctrl(controlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Control data broken.");
        return false;
    }

    if (!SaveUtils::isOwctrl(targetParentControlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Parent control data broken.");
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

    repairIds(newControlRootPath, true);

    SaveUtils::regenerateUids(newControlRootPath);

    return true;
}

bool SaveManager::moveControl(Control* control, const Control* parentControl)
{
    if (!SaveUtils::isOwctrl(control->dir()) || !SaveUtils::isOwctrl(parentControl->dir())) {
        qWarning("SaveManager::moveControl: Failed. One or more control data broken.");
        return false;
    }

    if (QString::compare(SaveUtils::toParentDir(control->dir()),
                         parentControl->dir(), Qt::CaseInsensitive) == 0) {
        return true;
    }

    const QString& targetControlChildrenDir = SaveUtils::toChildrenDir(parentControl->dir());
    const QString& newControlRootPath = targetControlChildrenDir + separator()
            + QString::number(SaveUtils::biggestDir(targetControlChildrenDir) + 1);

    if (!mkdir(newControlRootPath)) {
        qWarning("SaveManager::moveControl: Failed. Cannot create the new control root path.");
        return false;
    }

    if (!cp(control->dir(), newControlRootPath, true)) {
        qWarning("SaveManager::moveControl: Failed. Cannot copy the control to its new root path.");
        return false;
    }

    if (!rm(control->dir())) {
        qWarning("SaveManager::removeControl: Removal failed.");
        return false;
    }

    repairIds(newControlRootPath, true);

    for (Control* child : control->childControls()) {
        child->setUrl(child->url().replace(control->dir(), newControlRootPath, Qt::CaseInsensitive));
        child->setId(SaveUtils::id(child->dir()));
    }
    control->setUrl(SaveUtils::toUrl(newControlRootPath));
    control->setId(SaveUtils::id(control->dir()));

    return true;
}

void SaveManager::removeControl(const QString& rootPath)
{
    if (!SaveUtils::isOwctrl(rootPath)) {
        qWarning("SaveManager::removeControl: Failed. Control data broken.");
        return;
    }

    if (!rm(rootPath)) {
        qWarning("SaveManager::removeControl: Removal failed.");
        return;
    }
}

/*!
    Empty ids corrected
    Non-unique ids made unique
    Id conflicts on main.qml and control meta files fixed (the id of the control meta file takes precedence)
*/
void SaveManager::repairIds(const QString& rootPath, bool recursive)
{
    QStringList controlRootPaths;
    controlRootPaths.append(rootPath);

    if (recursive)
        controlRootPaths.append(SaveUtils::childrenPaths(rootPath));

    const QString& formRootPath = detectedFormRootPath(rootPath);

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

/*!
    NOTE: Do not use this directly from anywhere, use ControlPropertyManager instead
*/
void SaveManager::setProperty(Control* control, const QString& property, const QString& value)
{
    if (!control)
        return;

    if (!SaveUtils::isOwctrl(control->dir()))
        return;

    QString copy(value);

    if (property == "id") {
        if (control->id() == copy)
            return;

        /*!
            NOTE: We don't have to call ControlPropertyManager::setId in order to emit
                  idChanged signal in ControlPropertyManager; because setProperty cannot be
                  used anywhere else except ControlPropertyManager Hence,
                  setProperty(.., "id", ...) is only called from ControlPropertyManager::setId,
                  which means idChanged signal is already emitted.
        */

        ParserUtils::setId(control->url(), copy);
        repairIds(control->dir(), false);
        copy = SaveUtils::id(control->dir());
        control->setId(copy);
    } else {
        ParserUtils::setProperty(control->url(), property, copy);
    }

    ProjectManager::updateLastModification(ProjectManager::hash());

    instance()->propertyChanged(control, property, copy);
}