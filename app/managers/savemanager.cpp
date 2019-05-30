#include <savemanager.h>
#include <saveutils.h>
#include <hashfactory.h>
#include <projectmanager.h>
#include <parserutils.h>
#include <zipasync.h>
#include <control.h>
#include <utilityfunctions.h>
#include <filesystemutils.h>
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
        const QString& formId = ParserUtils::id(formRootPath);
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
        const QString& controlId = ParserUtils::id(controlRootPath);
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
    Q_ASSERT(!rootPath.isEmpty());
    Q_ASSERT(ProjectManager::isStarted());
    const QDir designs(SaveUtils::toProjectDesignsDir(ProjectManager::dir()));
    QDir root(rootPath);
    Q_ASSERT(root != designs);
    QDir deeper;
    do {
        deeper = root;
    } while(root.cdUp() && root != designs);

    const QString& formRootPath = deeper.path();
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
        const QString& idOrig = ParserUtils::id(controlRootPath);

        QString id(idOrig);
        if (id.isEmpty())
            id = "control";

        while (countIdInProjectFormScope(id, formRootPath) > (id == idOrig ? 1 : 0))
            id = UtilityFunctions::increasedNumberedText(id, false, true);

        ParserUtils::setId(controlRootPath, id);
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

void SaveManager::setupFormConnections(const QString& formRootPath)
{
    Q_ASSERT(SaveUtils::isControlValid(formRootPath));

    const QString& id = ParserUtils::id(formRootPath);
    Q_ASSERT(!id.isEmpty());

    QFile file(SaveUtils::toControlMainQmlFile(formRootPath));
    if (!file.open(QFile::ReadWrite)) {
        qWarning("SaveManager: Cannot open main qml file");
        return;
    }

    QByteArray content = file.readAll();
    Q_ASSERT(!content.isEmpty());

    QString FormJS(id + "JS");
    FormJS.replace(0, 1, FormJS[0].toUpper());
    content.replace("// AssetsHere",
                    QString::fromUtf8("Component.onCompleted: %1.%2_onCompleted()").arg(FormJS).arg(id).toUtf8());
    file.resize(0);
    file.write(content);
    file.close();

    const QString& assetsJSPath = SaveUtils::toProjectAssetsDir(ProjectManager::dir()) + '/' + id + ".js";
    file.setFileName(":/resources/other/form.js");
    if (!file.open(QFile::ReadOnly)) {
        qWarning("SaveManager: Cannot open :/resources/other/form.js");
        return;
    }
    QString js = file.readAll();
    file.close();
    js = js.arg(id);
    if (!QFileInfo::exists(assetsJSPath)) {
        file.setFileName(assetsJSPath);
        if (!file.open(QFile::WriteOnly)) {
            qWarning("SaveManager: Cannot open assetsJSPath");
            return;
        }
        file.write(js.toUtf8());
        file.close();
    } else {
        qWarning("SaveManager::setupFormConnections: Assets %s file is already exists.",
                 (id + ".js").toUtf8().constData());
        return;
    }

    emit instance()->formConnectionsDone(FormJS, id);
}

QString SaveManager::addForm(const QString& formRootPath)
{
    Q_ASSERT(ProjectManager::isStarted());

    if (!SaveUtils::isControlValid(formRootPath)) {
        qWarning("SaveManager::addForm: Failed. Form data broken.");
        return {};
    }

    const QString& targetDesignsDir = SaveUtils::toProjectDesignsDir(ProjectManager::dir());
    const QString& newFormRootPath = targetDesignsDir + '/' + HashFactory::generate();

    if (!QDir(newFormRootPath).mkpath(".")) {
        qWarning("SaveManager::addForm: Failed. Cannot create the new form root path.");
        return {};
    }

    if (!FileSystemUtils::copy(formRootPath, newFormRootPath, true)) {
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

    const QString& targetParentControlChildrenDir = SaveUtils::toControlChildrenDir(targetParentControlRootPath);
    const QString& newControlRootPath = targetParentControlChildrenDir + '/' + HashFactory::generate();

    if (!QDir(newControlRootPath).mkpath(".")) {
        qWarning("SaveManager::addControl: Failed. Cannot create the new control root path.");
        return {};
    }

    if (!FileSystemUtils::copy(controlRootPath, newControlRootPath, true)) {
        qWarning("SaveManager::addControl: Failed. Cannot copy the control to its new root path.");
        return {};
    }

    repairIds(newControlRootPath, true);

    SaveUtils::regenerateUids(newControlRootPath);

    return newControlRootPath;
}

bool SaveManager::moveControl(Control* control, const Control* parentControl)
{
    if (control == parentControl)
        return false;

    if (QString::compare(control->dir(), parentControl->dir(), Qt::CaseInsensitive) == 0)
        return false;

    if (!SaveUtils::isControlValid(control->dir()) || !SaveUtils::isControlValid(parentControl->dir())) {
        qWarning("SaveManager::moveControl: Failed. One or more control data broken.");
        return false;
    }

    if (QString::compare(SaveUtils::toDoubleUp(control->dir()),
                         parentControl->dir(), Qt::CaseInsensitive) == 0) {
        return true;
    }

    const QString& targetControlChildrenDir = SaveUtils::toControlChildrenDir(parentControl->dir());
    const QString& newControlRootPath = targetControlChildrenDir + '/' + QDir(control->dir()).dirName();

    if (!QDir(newControlRootPath).mkpath(".")) {
        qWarning("SaveManager::moveControl: Failed. Cannot create the new control root path.");
        return false;
    }

    if (!FileSystemUtils::copy(control->dir(), newControlRootPath, true)) {
        qWarning("SaveManager::moveControl: Failed. Cannot copy the control to its new root path.");
        return false;
    }

    if (!QDir(control->dir()).removeRecursively()) {
        qWarning("SaveManager::removeControl: Removal failed.");
        return false;
    }

    repairIds(newControlRootPath, true);

    for (Control* child : control->childControls()) {
        child->setDir(child->dir().replace(control->dir(), newControlRootPath, Qt::CaseInsensitive));
        const QString& childId = ParserUtils::id(control->dir());
        Q_ASSERT(!childId.isEmpty());
        child->setId(childId);
    }
    control->setDir(newControlRootPath);
    const QString& controlId = ParserUtils::id(control->dir());
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

    if (!QDir(rootPath).removeRecursively()) {
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

    if (!QDir(formRootPath).removeRecursively()) {
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

        ParserUtils::setId(control->dir(), copy);
        repairIds(control->dir(), false);
        copy = ParserUtils::id(control->dir());
        control->setId(copy);
        Q_ASSERT(!copy.isEmpty());
    } else {
        ParserUtils::setProperty(control->dir(), property, copy);
    }

    ProjectManager::updateLastModification(ProjectManager::uid());

    instance()->propertyChanged(control, property, copy);
}