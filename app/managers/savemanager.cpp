#include <savemanager.h>
#include <saveutils.h>
#include <hashfactory.h>
#include <projectmanager.h>
#include <parserutils.h>
#include <zipasync.h>
#include <control.h>
#include <utilityfunctions.h>
#include <QRegularExpression>

// TODO: Change QString with QLatin1String whenever it is possible
// TODO: Always use case insensitive comparison when it is possible
// TODO: Make a performance test over file system and Qml Parser usage, and improve them

namespace {

int countIdInProjectForms(const QString& id)
{
    Q_ASSERT(!id.isEmpty());

    int counter = 0;
    for (const QString& formRootPath : SaveUtils::formPaths(ProjectManager::dir())) {
        const QString& formId = ParserUtils::id(SaveUtils::toMainQmlFile(formRootPath));
        if (formId.isEmpty()) {
            qWarning("SaveManager::countIdInProjectForms: Empty id detected! Control: %s",
                     formRootPath.toUtf8().constData());
        }
        if (formId == id)
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
        const QString& controlId = ParserUtils::id(SaveUtils::toMainQmlFile(controlRootPath));
        if (controlId.isEmpty()) {
            qWarning("SaveManager::ccountIdInControlChildrenScope: Empty id detected! Control: %s",
                     controlRootPath.toUtf8().constData());
        }
        if (controlId == id)
            ++counter;
    }

    return counter;
}

inline int countIdInProjectFormScope(const QString& id, const QString formRootPath)
{
    return countIdInProjectForms(id) + countIdInControlChildrenScope(id, formRootPath);
}

QString detectedFormRootPath(const QString& rootPath)
{
    //! FIXME: This might crash on Windows due to back-slash path names
    Q_ASSERT(!ProjectManager::dir().isEmpty());
    const QString& designsDir = SaveUtils::toDesignsDir(ProjectManager::dir()) + '/';
    const QString& formRootPath = QRegularExpression("^" + designsDir + "\\d+").match(rootPath).captured();
    Q_ASSERT(!formRootPath.isEmpty());
    Q_ASSERT(QFileInfo::exists(formRootPath));
    Q_ASSERT(SaveUtils::isForm(formRootPath));
    return formRootPath;
}

/*!
    Empty ids corrected
    Non-unique ids made unique
    Id conflicts on main.qml and control meta files fixed (the id of the control meta file takes precedence)
*/
void repairIds(const QString& rootPath, bool recursive)
{
    QStringList controlRootPaths;
    controlRootPaths.append(rootPath);

    if (recursive)
        controlRootPaths.append(SaveUtils::childrenPaths(rootPath));

    const QString& formRootPath = detectedFormRootPath(rootPath);

    for (const QString& controlRootPath : controlRootPaths) {
        const QString& idOrig = ParserUtils::id(SaveUtils::toMainQmlFile(controlRootPath));

        QString id(idOrig);
        if (id.isEmpty())
            id = "control";

        while (countIdInProjectFormScope(id, formRootPath) > (id == idOrig ? 1 : 0))
            id = UtilityFunctions::increasedNumberedText(id, false, true);

        ParserUtils::setId(SaveUtils::toMainQmlFile(controlRootPath), id);
    }
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
            !QFileInfo::exists(projectDirectory) ||
            QFileInfo::exists(SaveUtils::toDesignsDir(projectDirectory)) ||
            !ZipAsync::unzipSync(":/templates/template" + QString::number(templateNumber) + ".zip",
                                projectDirectory)) {
        return false;
    }

    SaveUtils::regenerateUids(SaveUtils::toDesignsDir(projectDirectory));

    return true;
}

void SaveManager::setupFormGlobalConnections(const QString& formRootPath)
{
    Q_ASSERT(SaveUtils::isControlValid(formRootPath));

    const QString& id = ParserUtils::id(SaveUtils::toMainQmlFile(formRootPath));
    Q_ASSERT(!id.isEmpty());

    QByteArray content = rdfile(SaveUtils::toMainQmlFile(formRootPath));
    Q_ASSERT(!content.isEmpty());

    QString FormJS(id + "JS");
    FormJS.replace(0, 1, FormJS[0].toUpper());
    content.replace("// GlobalConnectionHere",
                    QString::fromUtf8("Component.onCompleted: %1.%2_onCompleted()").arg(FormJS).arg(id).toUtf8());
    wrfile(SaveUtils::toMainQmlFile(formRootPath), content);

    const QString& globalJSPath = SaveUtils::toGlobalDir(ProjectManager::dir()) + '/' + id + ".js";
    QString js = rdfile(":/resources/other/form.js");
    js = js.arg(id);
    if (!QFileInfo::exists(globalJSPath)) {
        wrfile(globalJSPath, js.toUtf8());
    } else {
        qWarning("SaveManager::setupFormGlobalConnections: Global %s file is already exists.",
                 (id + ".js").toUtf8().constData());
        return;
    }

    emit instance()->formGlobalConnectionsDone(FormJS, id);
}

QString SaveManager::addForm(const QString& formRootPath)
{
    Q_ASSERT(!ProjectManager::dir().isEmpty());

    if (!SaveUtils::isControlValid(formRootPath)) {
        qWarning("SaveManager::addForm: Failed. Form data broken.");
        return {};
    }

    const QString& targetDesignsDir = SaveUtils::toDesignsDir(ProjectManager::dir());
    const QString& newFormRootPath = targetDesignsDir + '/' + HashFactory::generate();

    if (!mkdir(newFormRootPath)) {
        qWarning("SaveManager::addForm: Failed. Cannot create the new form root path.");
        return {};
    }

    if (!cp(formRootPath, newFormRootPath, true)) {
        qWarning("SaveManager::addForm: Failed. Cannot copy the control to its new root path.");
        return {};
    }

    repairIds(newFormRootPath, true);

    SaveUtils::regenerateUids(newFormRootPath);

    return newFormRootPath;
}

QString SaveManager::addControl(const QString& controlRootPath, const QString& targetParentControlRootPath)
{
    if (!SaveUtils::isControlValid(controlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Control data broken.");
        return {};
    }

    if (!SaveUtils::isControlValid(targetParentControlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Parent control data broken.");
        return {};
    }

    const QString& targetParentControlChildrenDir = SaveUtils::toChildrenDir(targetParentControlRootPath);
    const QString& newControlRootPath = targetParentControlChildrenDir + '/' + HashFactory::generate();

    if (!mkdir(newControlRootPath)) {
        qWarning("SaveManager::addControl: Failed. Cannot create the new control root path.");
        return {};
    }

    if (!cp(controlRootPath, newControlRootPath, true)) {
        qWarning("SaveManager::addControl: Failed. Cannot copy the control to its new root path.");
        return {};
    }

    repairIds(newControlRootPath, true);

    SaveUtils::regenerateUids(newControlRootPath);

    return newControlRootPath;
}

bool SaveManager::moveControl(Control* control, const Control* parentControl)
{
    if (!SaveUtils::isControlValid(control->dir()) || !SaveUtils::isControlValid(parentControl->dir())) {
        qWarning("SaveManager::moveControl: Failed. One or more control data broken.");
        return false;
    }

    if (QString::compare(SaveUtils::toParentDir(control->dir()),
                         parentControl->dir(), Qt::CaseInsensitive) == 0) {
        return true;
    }

    const QString& targetControlChildrenDir = SaveUtils::toChildrenDir(parentControl->dir());
    const QString& newControlRootPath = targetControlChildrenDir + '/' + HashFactory::generate();

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
        const QString& childId = ParserUtils::id(child->url());
        Q_ASSERT(!childId.isEmpty());
        child->setId(childId);
    }
    control->setUrl(SaveUtils::toMainQmlFile(newControlRootPath));
    const QString& controlId = ParserUtils::id(control->url());
    Q_ASSERT(!controlId.isEmpty());
    control->setId(controlId);

    return true;
}

void SaveManager::removeControl(const QString& rootPath)
{
    if (!SaveUtils::isControlValid(rootPath)) {
        qWarning("SaveManager::removeControl: Failed. Control data broken.");
        return;
    }

    if (!rm(rootPath)) {
        qWarning("SaveManager::removeControl: Removal failed.");
        return;
    }
}

void SaveManager::removeForm(const QString& formRootPath)
{
    if (!SaveUtils::isControlValid(formRootPath)) {
        qWarning("SaveManager::removeForm: Failed. Either form data broken, or it is a main form.");
        return;
    }

    if (!rm(formRootPath)) {
        qWarning("SaveManager::removeForm: Removal failed.");
        return;
    }
}

/*!
    NOTE: Do not use this directly from anywhere, use ControlPropertyManager instead
*/
void SaveManager::setProperty(Control* control, const QString& property, const QString& value)
{
    if (!control)
        return;

    if (!SaveUtils::isControlValid(control->dir()))
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
        copy = ParserUtils::id(control->url());
        control->setId(copy);
        Q_ASSERT(!copy.isEmpty());
    } else {
        ParserUtils::setProperty(control->url(), property, copy);
    }

    ProjectManager::updateLastModification(ProjectManager::uid());

    instance()->propertyChanged(control, property, copy);
}