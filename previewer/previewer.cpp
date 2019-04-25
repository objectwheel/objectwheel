#include <previewer.h>
#include <commandlineparser.h>
#include <saveutils.h>
#include <parserutils.h>
#include <previewerutils.h>
#include <filemanager.h>
#include <previewresult.h>
#include <utilityfunctions.h>
#include <components.h>

#include <private/qqmlengine_p.h>

#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QPainter>
#include <QTimer>
#include <QApplication>

namespace {
qreal g_progress = 0;
qreal g_progressPerInstance = 0;
const int g_progress_1 = 2;
const int g_progress_2 = 6;
const int g_progress_3 = 10;
const int g_progress_4 = 100;
}

Previewer::Previewer(QObject* parent) : QObject(parent)
  , m_initialized(false)
  , m_view(new QQuickView)
{
    DesignerSupport::createOpenGLContext(m_view);
    DesignerSupport::setRootItem(m_view, PreviewerUtils::createDummyItem(m_view->engine()));
    m_view->engine()->setOutputWarningsToStandardError(false);
    m_view->engine()->addImportPath(SaveUtils::toImportsDir(CommandlineParser::projectDirectory()));
    m_view->engine()->addImportPath(SaveUtils::toGlobalDir(CommandlineParser::projectDirectory()));
}

Previewer::~Previewer()
{
    PreviewerUtils::cleanUpFormInstances(m_formInstances, m_view->rootContext(), m_designerSupport);
    delete m_view;
}

void Previewer::init()
{
    emit initializationProgressChanged(g_progress_1);

    /* Create instances, handle parent-child relationship, set ids, save form instances */
    QMap<QString, ControlInstance*> instanceTree;
    for (const QString& formPath : SaveUtils::formPaths(CommandlineParser::projectDirectory())) {
        ControlInstance* formInstance = createInstance(formPath, nullptr);
        Q_ASSERT(formInstance);

        m_formInstances.append(formInstance);
        instanceTree.insert(formPath, formInstance);

        // TODO: What if a child is a master-control?
        for (const QString& childPath : SaveUtils::childrenPaths(formPath)) {
            ControlInstance* parentInstance = instanceTree.value(SaveUtils::toParentDir(childPath));
            Q_ASSERT(parentInstance);
            ControlInstance* childInstance = createInstance(childPath, parentInstance);
            Q_ASSERT(childInstance);
            instanceTree.insert(childPath, childInstance);
        }
    }

    emit initializationProgressChanged(g_progress_2);

    // FIXME: Object completetion order must be parent -> to -> child
    for (ControlInstance* instance : instanceTree.values())
        PreviewerUtils::doComplete(instance, this);

    for (ControlInstance* instance : instanceTree.values()) {
        if (QQuickItem* item = PreviewerUtils::guiItem(instance))
            DesignerSupport::addDirty(item, DesignerSupport::AllMask);
    }

    /* Refresh expressions */
    refreshAllBindings();

    emit initializationProgressChanged(g_progress_3);

    /* Preview */
    int totalInstanceCount = 0;
    for (Previewer::ControlInstance* formInstance : m_formInstances)
        totalInstanceCount += PreviewerUtils::countAllSubInstance(formInstance);
    g_progressPerInstance = qreal(g_progress_4 - g_progress_3) / totalInstanceCount;
    g_progress = g_progress_3;
    for (ControlInstance* formInstance : m_formInstances)
        schedulePreview(formInstance);
}

void Previewer::updateProperty(const QString& uid, const QString& propertyName, const QVariant& propertyValue)
{
    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->object);
    Q_ASSERT(instance->errors.isEmpty());

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    PreviewerUtils::setInstancePropertyVariant(instance, propertyName, propertyValue);

    refreshBindings(formInstance->context);
    schedulePreview(formInstance);
}

// BUG: Inter form reparents have problems, cause context of the moved control will still remain
// pointing to old forms context.
void Previewer::updateParent(const QString& newDir, const QString& uid, const QString& parentUid)
{
    Q_ASSERT_X(!SaveUtils::isForm(newDir), "parentUpdate", "You can't change parent of a form.");

    ControlInstance* instance = instanceForUid(uid);
    ControlInstance* parentInstance = instanceForUid(parentUid);

    Q_ASSERT(instance);
    Q_ASSERT(parentInstance);

    ControlInstance* formInstance = formInstanceFor(parentInstance);
    ControlInstance* previousParentInstance = instance->parent;

    Q_ASSERT(formInstance);
    Q_ASSERT(previousParentInstance);

    instance->dir = newDir;
    instance->parent = parentInstance;
    parentInstance->children.append(instance);
    previousParentInstance->children.removeAll(instance);

    if (!instance->errors.isEmpty())
        return;

    QObject* parentObject;
    if (parentInstance->errors.isEmpty()) {
        if (parentInstance->gui)
            parentObject = parentInstance->object;
        else
            parentObject = m_view->rootObject();
    } else {
        parentObject = m_view->rootObject();
    }

    QQmlProperty defaultProperty(parentObject);
    Q_ASSERT(defaultProperty.isValid());

    QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
    childList.append(instance->object);

    if (instance->window || instance->popup) { // We still reparent it anyway, may a window comes
        QQuickItem* item = PreviewerUtils::guiItem(instance->object);
        item->setParentItem(PreviewerUtils::guiItem(parentObject));
    }

    refreshBindings(formInstance->context);
    schedulePreview(formInstance);
}

void Previewer::updateControlCode(const QString& uid)
{
    ControlInstance* oldInstance = instanceForUid(uid);

    Q_ASSERT(oldInstance);
    Q_ASSERT(oldInstance->parent);

    Q_ASSERT_X(!SaveUtils::isForm(oldInstance->dir),
               "updateControlCode", "You can't change code of a form here.");

    ControlInstance* formInstance = formInstanceFor(oldInstance);

    Q_ASSERT(formInstance);

    QObject* oldObject = oldInstance->object;

    QQuickItem* item = PreviewerUtils::guiItem(oldObject);
    if (item)
        m_designerSupport.derefFromEffectItem(item);

    PreviewerUtils::setId(oldInstance->context, nullptr, oldInstance->id, QString());

    ControlInstance* instance = createInstance(oldInstance->dir, oldInstance->parent);
    oldInstance->gui = instance->gui;
    oldInstance->popup = instance->popup;
    oldInstance->window = instance->window;
    oldInstance->codeChanged = instance->codeChanged;
    oldInstance->id = instance->id;
    oldInstance->object = instance->object;
    oldInstance->errors = instance->errors;
    oldInstance->parent->children.removeAll(instance);
    if (!instance->gui) { // Error condition is also included
        m_dirtyInstanceSet.remove(instance);
        m_dirtyInstanceSet.insert(oldInstance);
    }
    delete instance;

    for (ControlInstance* childInstance : oldInstance->children) {
        if (oldInstance->errors.isEmpty()) {
            if (oldInstance->gui) {
                if (childInstance->errors.isEmpty()) {
                    QQmlProperty defaultProperty(oldInstance->object);
                    Q_ASSERT(defaultProperty.isValid());

                    QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
                    childList.append(childInstance->object);

                    if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                        QQuickItem* item = PreviewerUtils::guiItem(childInstance->object);
                        item->setParentItem(PreviewerUtils::guiItem(oldInstance->object));
                    }
                }
            } else {
                if (childInstance->errors.isEmpty()) {
                    QQmlProperty defaultProperty(m_view->rootObject());
                    Q_ASSERT(defaultProperty.isValid());

                    QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
                    childList.append(childInstance->object);

                    if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                        QQuickItem* item = PreviewerUtils::guiItem(childInstance->object);
                        item->setParentItem(PreviewerUtils::guiItem(m_view->rootObject()));
                    }
                }
            }
        } else {
            if (childInstance->errors.isEmpty()) {
                QQmlProperty defaultProperty(m_view->rootObject());
                Q_ASSERT(defaultProperty.isValid());

                QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
                childList.append(childInstance->object);

                if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                    QQuickItem* item = PreviewerUtils::guiItem(childInstance->object);
                    item->setParentItem(PreviewerUtils::guiItem(m_view->rootObject()));
                }
            }
        }
    }

    // We delete previous instance object after we reparent all of its children into the new instance
    if (oldObject)
        delete oldObject;


    PreviewerUtils::doComplete(oldInstance, this);

    refreshBindings(formInstance->context);
    schedulePreview(formInstance);
}

void Previewer::updateFormCode(const QString& uid)
{
    ControlInstance* oldFormInstance = instanceForUid(uid);

    Q_ASSERT(oldFormInstance);
    Q_ASSERT_X(SaveUtils::isForm(oldFormInstance->dir),
               "updateFormCode", "You can't change code of a control here.");

    QObject* oldObject = oldFormInstance->object;

    PreviewerUtils::setId(m_view->rootContext(), nullptr, oldFormInstance->id, QString());
    PreviewerUtils::setId(oldFormInstance->context, nullptr, oldFormInstance->id, QString());

    QQuickItem* item = PreviewerUtils::guiItem(oldObject);

    if (item)
        m_designerSupport.derefFromEffectItem(item);

    ControlInstance* instance = createInstance(oldFormInstance->dir, nullptr, oldFormInstance->context);
    oldFormInstance->gui = instance->gui;
    oldFormInstance->popup = instance->popup;
    oldFormInstance->window = instance->window;
    oldFormInstance->codeChanged = instance->codeChanged;
    oldFormInstance->id = instance->id;
    oldFormInstance->object = instance->object;
    oldFormInstance->errors = instance->errors;
    if (!instance->gui) { // Error condition is also included
        m_dirtyInstanceSet.remove(instance);
        m_dirtyInstanceSet.insert(oldFormInstance);
    }
    delete instance;

    for (ControlInstance* childInstance : oldFormInstance->children) {
        if (oldFormInstance->errors.isEmpty()) {
            Q_ASSERT(oldFormInstance->gui);
            if (childInstance->errors.isEmpty()) {
                QQmlProperty defaultProperty(oldFormInstance->object);
                Q_ASSERT(defaultProperty.isValid());

                QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
                childList.append(childInstance->object);

                if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                    QQuickItem* item = PreviewerUtils::guiItem(childInstance->object);
                    item->setParentItem(PreviewerUtils::guiItem(oldFormInstance->object));
                }
            }
        } else {
            if (childInstance->errors.isEmpty()) {
                QQmlProperty defaultProperty(m_view->rootObject());
                Q_ASSERT(defaultProperty.isValid());

                QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
                childList.append(childInstance->object);

                if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                    QQuickItem* item = PreviewerUtils::guiItem(childInstance->object);
                    item->setParentItem(PreviewerUtils::guiItem(m_view->rootObject()));
                }
            }
        }
    }

    if (oldObject)
        delete oldObject;

    PreviewerUtils::doComplete(oldFormInstance, this);

    // NOTE: We would normally wait until preview() function to detect if there are any dirts
    // But since we have a "margin" problem, all the (first) sub-childs should be updated in the first place
    // In order to let chilren to know if there were any "header", "footer" (therefore margin) changes.
    m_dirtyInstanceSet.insert(oldFormInstance);
    for (ControlInstance* instance : oldFormInstance->children)
        m_dirtyInstanceSet.insert(instance);

    refreshAllBindings();
    schedulePreview(oldFormInstance);
}

void Previewer::refreshAllBindings()
{
    DesignerSupport::refreshExpressions(m_view->rootContext());
    for (ControlInstance* formInstance : m_formInstances)
        DesignerSupport::refreshExpressions(formInstance->context);
    DesignerSupport::refreshExpressions(m_view->rootContext());
}

void Previewer::updateId(const QString& uid, const QString& newId)
{
    Q_ASSERT(!newId.isEmpty());

    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->object);
    Q_ASSERT(instance->errors.isEmpty());

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    if (formInstance == instance)
        PreviewerUtils::setId(m_view->rootContext(), instance->object, instance->id, newId);
    PreviewerUtils::setId(instance->context, instance->object, instance->id, newId);

    instance->id = newId;

    refreshBindings(formInstance->context);
    schedulePreview(formInstance);
}

void Previewer::createControl(const QString& dir, const QString& parentUid)
{
    ControlInstance* parentInstance = instanceForUid(parentUid);

    Q_ASSERT(parentInstance);

    ControlInstance* formInstance = formInstanceFor(parentInstance);

    Q_ASSERT(formInstance);

    ControlInstance* instance = createInstance(dir, parentInstance);

    PreviewerUtils::doComplete(instance, this);

    refreshBindings(formInstance->context);
    schedulePreview(formInstance);
}

void Previewer::createForm(const QString& dir)
{
    ControlInstance* formInstance = createInstance(dir, nullptr);

    m_formInstances.append(formInstance);
    PreviewerUtils::doComplete(formInstance, this);

    refreshAllBindings();
    schedulePreview(formInstance);
}

void Previewer::deleteForm(const QString& uid)
{
    ControlInstance* formInstance = instanceForUid(uid);

    Q_ASSERT(formInstance);

    m_formInstances.removeAll(formInstance);
    PreviewerUtils::cleanUpFormInstances(QList<ControlInstance*>{formInstance},
                                         m_view->rootContext(), m_designerSupport);

    refreshAllBindings();

    for (ControlInstance* formInstance : m_formInstances)
        schedulePreview(formInstance);
}

void Previewer::deleteControl(const QString& uid)
{
    Q_ASSERT(!uid.isEmpty());
    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->parent);

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    PreviewerUtils::deleteInstancesRecursive(instance, m_designerSupport);
    instance->parent->children.removeAll(instance);

    refreshBindings(formInstance->context);
    schedulePreview(formInstance);
}

void Previewer::refresh(const QString& formUid)
{
    Q_ASSERT(!formUid.isEmpty());
    ControlInstance* formInstance = instanceForUid(formUid);

    Q_ASSERT(formInstance);

    PreviewerUtils::makeDirtyRecursive(formInstance);

    refreshBindings(formInstance->context);
    schedulePreview(formInstance);
}

bool Previewer::hasInstanceForObject(const QObject* object) const
{
    Q_ASSERT(object);
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : PreviewerUtils::allSubInstance(formInstance)) {
            if (instance->object == object)
                return true;
            if (instance->window && PreviewerUtils::guiItem(instance) == object)
                return true;
            if (instance->popup && PreviewerUtils::guiItem(instance) == object)
                return true;
        }
    }
    return false;
}

bool Previewer::hasInstanceForUid(const QString& uid) const
{
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : PreviewerUtils::allSubInstance(formInstance)) {
            if (instance->uid == uid)
                return true;
        }
    }

    return false;
}

Previewer::ControlInstance* Previewer::instanceForObject(const QObject* object) const
{
    Q_ASSERT(object);
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : PreviewerUtils::allSubInstance(formInstance)) {
            if (instance->object == object)
                return instance;
            if (instance->window && PreviewerUtils::guiItem(instance) == object)
                return instance;
            if (instance->popup && PreviewerUtils::guiItem(instance) == object)
                return instance;
        }
    }
    return nullptr;
}

Previewer::ControlInstance* Previewer::instanceForUid(const QString& uid) const
{
    Q_ASSERT(!uid.isEmpty());
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : PreviewerUtils::allSubInstance(formInstance)) {
            if (instance->uid == uid)
                return instance;
        }
    }

    return nullptr;
}

Previewer::ControlInstance* Previewer::formInstanceFor(const Previewer::ControlInstance* instance)
{
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* childInstance : PreviewerUtils::allSubInstance(formInstance)) {
            if (instance == childInstance)
                return formInstance;
        }
    }

    return nullptr;
}

Previewer::ControlInstance* Previewer::findNodeInstanceForItem(QQuickItem* item) const
{
    if (item) {
        if (hasInstanceForObject(item))
            return instanceForObject(item);
        else if (item->parentItem())
            return findNodeInstanceForItem(item->parentItem());
    }

    return nullptr;
}

void Previewer::schedulePreview(ControlInstance* formInstance, int msecLater)
{
    QTimer::singleShot(msecLater, std::bind(&Previewer::preview, this, formInstance));
}

void Previewer::scheduleRepreviewForInvisibleInstances(Previewer::ControlInstance* formInstance, int msecLater)
{
    QTimer::singleShot(msecLater, this, [=] {
        QList<ControlInstance*> repreviewInstances;
        for (ControlInstance* instance : PreviewerUtils::allSubInstance(formInstance)) {
            if (!instance->needsRepreview)
                continue;

            if (!instance->errors.isEmpty())
                continue;

            if (!instance->gui)
                continue;

            QQuickItem* item = PreviewerUtils::guiItem(instance);

            if (!item->isVisible())
                continue;

            if (instance->window
                    && (ParserUtils::property(SaveUtils::toMainQmlFile(instance->dir), "visible") != "true"
                        || ParserUtils::property(SaveUtils::toMainQmlFile(instance->dir), "visibility").contains("Hidden"))) {
                continue;
            }

            if (instance->popup && ParserUtils::property(SaveUtils::toMainQmlFile(instance->dir), "visible") == "false")
                continue;

            DesignerSupport::addDirty(item, DesignerSupport::AllMask);

            repreviewInstances.append(instance);
        }

        refreshBindings(formInstance->context);
        emit previewDone(previewDirtyInstances(repreviewInstances));
    });
}

void Previewer::preview(ControlInstance* formInstance)
{
    Q_ASSERT(formInstance);

    DesignerSupport::polishItems(m_view);

    static DesignerSupport::DirtyType dirty = DesignerSupport::DirtyType(
                DesignerSupport::TransformUpdateMask
                | DesignerSupport::ContentUpdateMask
                | DesignerSupport::Visible
                | DesignerSupport::ZValue
                | DesignerSupport::OpacityValue
                | DesignerSupport::AllMask);

    for (QQuickItem* item : PreviewerUtils::allItems(formInstance)) {
        if (item) {
            if (hasInstanceForObject(item)) {
                if (DesignerSupport::isDirty(item, dirty))
                    m_dirtyInstanceSet.insert(instanceForObject(item));
            } else if (DesignerSupport::isDirty(item, dirty)) {
                ControlInstance* ancestorInstance = findNodeInstanceForItem(item->parentItem());
                if (ancestorInstance)
                    m_dirtyInstanceSet.insert(ancestorInstance);
            }
            DesignerSupport::updateDirtyNode(item);
        }
    }

    if (!m_dirtyInstanceSet.isEmpty()) {
        emit previewDone(previewDirtyInstances(m_dirtyInstanceSet));
        if (m_initialized)
            scheduleRepreviewForInvisibleInstances(formInstance);
        m_dirtyInstanceSet.clear();
    }

    PreviewerUtils::resetAllItems(formInstance);

    if (!m_initialized) {
        static QList<ControlInstance*> initializedInstances;
        initializedInstances.append(formInstance);
        if (initializedInstances.size() == m_formInstances.size()) {
            emit initializationProgressChanged(g_progress_4);
            m_initialized = true;
            for (ControlInstance* formInstance : m_formInstances)
                scheduleRepreviewForInvisibleInstances(formInstance);
        }
    }
}

QList<PreviewResult> Previewer::previewDirtyInstances(const QList<Previewer::ControlInstance*>& instances)
{
    QList<PreviewResult> results;
    for (int i = 0; i < instances.size(); ++i) {
        ControlInstance* instance = instances.at(i);
        Q_ASSERT(instance);
        PreviewResult result;
        result.id = instance->id;
        result.uid = instance->uid;
        result.gui = instance->gui;
        result.popup = instance->popup;
        result.window = instance->window;
        result.codeChanged = instance->codeChanged;
        result.properties = PreviewerUtils::properties(instance);
        result.events = PreviewerUtils::events(instance);
        instance->codeChanged = false;
        result.errors = instance->errors;
        result.image = grabImage(instance);
        results.append(result);

        if (instance->errors.isEmpty()
                && instance->gui
                && PreviewerUtils::guiItem(instance)->isVisible())
            instance->needsRepreview = PreviewerUtils::needsRepreview(result.image);

        if (!m_initialized) {
            g_progress += g_progressPerInstance;
            if (g_progress <= g_progress_4)
                emit initializationProgressChanged(g_progress);
        }
    }

    return results;
}

QImage Previewer::grabImage(const Previewer::ControlInstance* instance)
{
    Q_ASSERT(instance);
    if (!instance->errors.isEmpty()) {
        return QImage();
    } else if (!instance->gui) {
        return QImage();
    } else {
        QColor winColor;
        QQuickItem* item;

        if (instance->window) {
            QQuickWindow* window = static_cast<QQuickWindow*>(instance->object);
            item = window->contentItem();
            winColor = window->color();
        } else if (instance->popup) {
            item = PreviewerUtils::guiItem(instance->object);
        } else {
            item = static_cast<QQuickItem*>(instance->object);
        }

        if (instance->window
                && (ParserUtils::property(SaveUtils::toMainQmlFile(instance->dir), "visible") != "true"
                    || ParserUtils::property(SaveUtils::toMainQmlFile(instance->dir), "visibility").contains("Hidden"))) {
            return QImage();
        } else if (instance->popup && ParserUtils::property(SaveUtils::toMainQmlFile(instance->dir), "visible") == "false") {
            return QImage();
        } else {
            if (item->isVisible())
                return renderItem(item, winColor);
            else
                return QImage();
        }
    }
}

QImage Previewer::renderItem(QQuickItem* item, const QColor& bgColor)
{
    Q_ASSERT(item);
    PreviewerUtils::updateDirtyNodesRecursive(item, this);

    QRectF renderBoundingRect = QRectF(QPointF(), item->size());
    QSize size = renderBoundingRect.size().toSize();
    size *= qApp->devicePixelRatio();

    QImage renderImage(size, QImage::Format_ARGB32_Premultiplied);
    renderImage.setDevicePixelRatio(qApp->devicePixelRatio());

    if (bgColor.isValid())
        renderImage.fill(bgColor);
    else
        renderImage.fill(Qt::transparent);

    QPainter painter(&renderImage);
    painter.drawImage(QRect{{0, 0}, size / qApp->devicePixelRatio()},
                      m_designerSupport.renderImageForItem(item, renderBoundingRect, size),
                      QRect{{0, 0}, size});

    return renderImage;
}

void Previewer::refreshBindings(QQmlContext* context)
{
    m_designerSupport.refreshExpressions(context);
    m_designerSupport.refreshExpressions(m_view->rootContext());
}

/*
    Creates control instance on engine's root context.

    Following properties are always set:

        bool gui;
        bool window;
        QString id;
        QString uid;
        QString dir;
        QObject* object;
        QList<QQmlError> errors;

    If item contains any error, its "gui" property is set to true and a dummy quick
    item is placed. And initial properties are also set by setInitialProperties().
    All the window objects are hid. Id property is also set on root context.
*/
Previewer::ControlInstance* Previewer::createInstance(const QString& dir,
                                                      ControlInstance* parentInstance,
                                                      QQmlContext* oldFormContext)
{
    Q_ASSERT_X(SaveUtils::isControlValid(dir), "createInstance", "Owctrl™ structure is corrupted.");

    ComponentCompleteDisabler disabler;
    Q_UNUSED(disabler)

    const QString& url = SaveUtils::toMainQmlFile(dir);

    auto instance = new Previewer::ControlInstance;
    instance->dir = dir;
    instance->id = SaveUtils::id(dir);
    instance->uid = SaveUtils::uid(dir);
    instance->codeChanged = true;

    Q_ASSERT(!instance->id.isEmpty());
    Q_ASSERT(!instance->uid.isEmpty());

    if (SaveUtils::isForm(dir)) {
        if (oldFormContext)
            instance->context = oldFormContext;
        else
            instance->context = new QQmlContext(m_view->engine());
    } else {
        Q_ASSERT(parentInstance);
        parentInstance->children.append(instance);
        instance->parent = parentInstance;
        instance->context = parentInstance->context;
    }

    m_view->engine()->clearComponentCache();
    QQmlComponent component(m_view->engine());
    component.loadUrl(QUrl::fromLocalFile(url));

    QObject* object = component.beginCreate(instance->context);

    if (component.isError()) {
        if (object)
            delete object;

        instance->gui = false;
        instance->popup = false;
        instance->window = false;
        instance->object = nullptr;
        instance->errors = component.errors();

        m_dirtyInstanceSet.insert(instance);
        return instance;
    }

    Q_ASSERT(object);
    Q_ASSERT(!object->isWindowType() || object->inherits("QQuickWindow"));

    PreviewerUtils::tweakObjects(object);
    component.completeCreate();
    // FIXME: what if the component is a Component qml type or crashing type? and other possibilities

    if (QQmlEngine::contextForObject(object) == nullptr)
        QQmlEngine::setContextForObject(object, instance->context);

    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    QQmlEnginePrivate::get(m_view->engine())->cache(object->metaObject());
    if (SaveUtils::isForm(dir))
        PreviewerUtils::setId(m_view->rootContext(), object, QString(), instance->id);
    PreviewerUtils::setId(instance->context, object, QString(), instance->id);

    instance->object = object;
    instance->popup = object->inherits("QQuickPopup");
    instance->window = object->isWindowType();
    instance->gui = instance->window || instance->popup || object->inherits("QQuickItem");

    /*!
        FIXME: Popup (from QtQuick.Controls 2.0) is based on QtObject type, hence it is not a gui
               object in theory, but it is. And if it comes, as a form, or anything that inherits
               Popup comes (Dialog i.e.) crashes here.
    */
    Q_ASSERT(!SaveUtils::isForm(dir) || instance->gui);

    if (!instance->gui)
        m_dirtyInstanceSet.insert(instance);

    if (SaveUtils::isForm(dir)) {
        QQuickItem* formItem = PreviewerUtils::guiItem(instance);
        formItem->setFlag(QQuickItem::ItemHasContents, true);
        static_cast<QQmlParserStatus*>(formItem)->classBegin();
        formItem->setParentItem(m_view->rootObject());
        m_designerSupport.refFromEffectItem(formItem);
        formItem->update();
    } else {
        QObject* parentObject;
        if (parentInstance->errors.isEmpty()) {
            if (parentInstance->gui)
                parentObject = parentInstance->object;
            else
                parentObject = m_view->rootObject();
        } else {
            parentObject = m_view->rootObject();
        }

        QQmlProperty defaultProperty(parentObject);
        Q_ASSERT(defaultProperty.isValid());

        if (instance->gui) {
            QQuickItem* item = PreviewerUtils::guiItem(instance->object);
            item->setFlag(QQuickItem::ItemHasContents, true);
            static_cast<QQmlParserStatus*>(item)->classBegin();
        }

        QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
        childList.append(instance->object);

        if (instance->gui) {
            QQuickItem* item = PreviewerUtils::guiItem(instance->object);
            if (instance->window || instance->popup)
                item->setParentItem(PreviewerUtils::guiItem(parentObject)); // We still reparent it anyway, may a window comes
            m_designerSupport.refFromEffectItem(item);
            item->update();
        }
    }

    return instance;
}
