#include <renderengine.h>
#include <commandlineparser.h>
#include <saveutils.h>
#include <renderutils.h>
#include <renderresult.h>
#include <utilityfunctions.h>
#include <components.h>
#include <paintutils.h>
#include <private/qquickdesignersupportmetainfo_p.h>

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

enum {
    RENDER_TIMEOUT = 50,
    RERENDER_TIMEOUT = 400
};

RenderEngine::RenderEngine(QObject* parent) : QObject(parent)
  , m_initialized(false)
  , m_devicePixelRatio(qgetenv("RENDERER_DEVICE_PIXEL_RATIO").toDouble())
  , m_view(new QQuickView)
  , m_renderTimer(new QTimer(this))
  , m_reRenderTimer(new QTimer(this))
{
    DesignerSupport::createOpenGLContext(m_view);
    DesignerSupport::setRootItem(m_view, RenderUtils::createDummyItem(m_view->engine()));

    m_view->engine()->setOutputWarningsToStandardError(false);
    m_view->engine()->addImportPath(SaveUtils::toProjectImportsDir(CommandlineParser::projectDirectory()));

    m_renderTimer->setInterval(RENDER_TIMEOUT);
    connect(m_renderTimer, &QTimer::timeout, this, &RenderEngine::flushRenders);

    m_reRenderTimer->setInterval(RERENDER_TIMEOUT);
    connect(m_reRenderTimer, &QTimer::timeout, this, &RenderEngine::flushReRenders);
}

RenderEngine::~RenderEngine()
{
    RenderUtils::cleanUpFormInstances(m_formInstances, m_view->rootContext(), m_designerSupport);
    delete m_view;
}

void RenderEngine::init()
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
            ControlInstance* parentInstance = instanceTree.value(SaveUtils::toDoubleUp(childPath));
            Q_ASSERT(parentInstance);
            ControlInstance* childInstance = createInstance(childPath, parentInstance);
            Q_ASSERT(childInstance);
            instanceTree.insert(childPath, childInstance);
        }
    }

    emit initializationProgressChanged(g_progress_2);

    // FIXME: Object completetion order must be parent -> to -> child
    // according to Qt's internal completion order read here:
    // stackoverflow.com/questions/46196831
    for (ControlInstance* instance : instanceTree)
        RenderUtils::doComplete(instance, this);

    for (ControlInstance* instance : instanceTree) {
        if (QQuickItem* item = RenderUtils::guiItem(instance))
            DesignerSupport::addDirty(item, DesignerSupport::AllMask);
    }

    /* Refresh expressions */
    refreshAllBindings();

    emit initializationProgressChanged(g_progress_3);

    /* Render */
    int totalInstanceCount = 0;
    for (RenderEngine::ControlInstance* formInstance : m_formInstances)
        totalInstanceCount += RenderUtils::countAllSubInstance(formInstance);
    g_progressPerInstance = qreal(g_progress_4 - g_progress_3) / totalInstanceCount;
    g_progress = g_progress_3;
    for (ControlInstance* formInstance : m_formInstances)
        scheduleRender(formInstance);
}

void RenderEngine::updateProperty(const QString& uid, const QString& propertyName, const QVariant& propertyValue)
{
    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->object);
    Q_ASSERT(instance->errors.isEmpty());

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    RenderUtils::setInstancePropertyVariant(instance, propertyName, propertyValue);

    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

// BUG: Inter-form reparents have problems, cause context of the moved control will still remain
// pointing to old forms context.
void RenderEngine::updateParent(const QString& newDir, const QString& uid, const QString& parentUid)
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
    Q_ASSERT(previousParentInstance != parentInstance);

    instance->dir = newDir;
    instance->parent = parentInstance;
    previousParentInstance->children.removeAll(instance);
    parentInstance->children.append(instance);

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
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        item->setParentItem(RenderUtils::guiItem(parentObject));
    }

    RenderUtils::refreshLayoutable(previousParentInstance);
    RenderUtils::refreshLayoutable(parentInstance);

    repairIndexes(previousParentInstance);
    repairIndexes(parentInstance);
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::updateControlCode(const QString& uid)
{
    ControlInstance* oldInstance = instanceForUid(uid);

    Q_ASSERT(oldInstance);
    Q_ASSERT(oldInstance->parent);

    Q_ASSERT_X(!SaveUtils::isForm(oldInstance->dir),
               "updateControlCode", "You can't change code of a form here.");

    ControlInstance* formInstance = formInstanceFor(oldInstance);

    Q_ASSERT(formInstance);

    QObject* oldObject = oldInstance->object;

    QQuickItem* item = RenderUtils::guiItem(oldObject);
    if (item)
        m_designerSupport.derefFromEffectItem(item);

    RenderUtils::setId(oldInstance->context, nullptr, oldInstance->id, QString());

    ControlInstance* instance = createInstance(oldInstance->dir, oldInstance->parent);
    oldInstance->gui = instance->gui;
    oldInstance->layout = instance->layout;
    oldInstance->popup = instance->popup;
    oldInstance->window = instance->window;
    oldInstance->visible = instance->visible;
    oldInstance->codeChanged = instance->codeChanged;
    oldInstance->geometryHash = instance->geometryHash;
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
                        QQuickItem* item = RenderUtils::guiItem(childInstance->object);
                        item->setParentItem(RenderUtils::guiItem(oldInstance->object));
                    }
                }
            } else {
                if (childInstance->errors.isEmpty()) {
                    QQmlProperty defaultProperty(m_view->rootObject());
                    Q_ASSERT(defaultProperty.isValid());

                    QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
                    childList.append(childInstance->object);

                    if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                        QQuickItem* item = RenderUtils::guiItem(childInstance->object);
                        item->setParentItem(RenderUtils::guiItem(m_view->rootObject()));
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
                    QQuickItem* item = RenderUtils::guiItem(childInstance->object);
                    item->setParentItem(RenderUtils::guiItem(m_view->rootObject()));
                }
            }
        }
    }

    // We delete previous instance object after we reparent all of its children into the new instance
    if (oldObject)
        delete oldObject;

    RenderUtils::doComplete(oldInstance, this);

    repairIndexes(oldInstance);
    repairIndexes(oldInstance->parent);
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::updateFormCode(const QString& uid)
{
    ControlInstance* oldFormInstance = instanceForUid(uid);

    Q_ASSERT(oldFormInstance);
    Q_ASSERT_X(SaveUtils::isForm(oldFormInstance->dir),
               "updateFormCode", "You can't change code of a control here.");

    QObject* oldObject = oldFormInstance->object;

    RenderUtils::setId(m_view->rootContext(), nullptr, oldFormInstance->id, QString());
    RenderUtils::setId(oldFormInstance->context, nullptr, oldFormInstance->id, QString());

    QQuickItem* item = RenderUtils::guiItem(oldObject);

    if (item)
        m_designerSupport.derefFromEffectItem(item);

    ControlInstance* instance = createInstance(oldFormInstance->dir, nullptr, oldFormInstance->context);
    oldFormInstance->gui = instance->gui;
    oldFormInstance->layout = instance->layout;
    oldFormInstance->popup = instance->popup;
    oldFormInstance->window = instance->window;
    oldFormInstance->visible = instance->visible;
    oldFormInstance->codeChanged = instance->codeChanged;
    oldFormInstance->geometryHash = instance->geometryHash;
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
                    QQuickItem* item = RenderUtils::guiItem(childInstance->object);
                    item->setParentItem(RenderUtils::guiItem(oldFormInstance->object));
                }
            }
        } else {
            if (childInstance->errors.isEmpty()) {
                QQmlProperty defaultProperty(m_view->rootObject());
                Q_ASSERT(defaultProperty.isValid());

                QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
                childList.append(childInstance->object);

                if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                    QQuickItem* item = RenderUtils::guiItem(childInstance->object);
                    item->setParentItem(RenderUtils::guiItem(m_view->rootObject()));
                }
            }
        }
    }

    if (oldObject)
        delete oldObject;

    RenderUtils::doComplete(oldFormInstance, this);

    // NOTE: We would normally wait until render() function to detect if there are any dirts
    // But since we have a "margin" problem, all the (first) sub-childs should be updated in the first place
    // In order to let chilren to know if there were any "header", "footer" (therefore margin) changes.
    m_dirtyInstanceSet.insert(oldFormInstance);
    for (ControlInstance* instance : oldFormInstance->children)
        m_dirtyInstanceSet.insert(instance);

    // Form indexes ignored, since they are put upon rootObject of the engine
    // So, no need this: repairIndexes(oldFormInstance->parent??);
    repairIndexes(oldFormInstance);
    refreshAllBindings();
    scheduleRender(oldFormInstance);
}

void RenderEngine::preview(const QString& url)
{
    Q_ASSERT(!url.isEmpty());

    ControlInstance* instance = createInstance(url);

    RenderUtils::doComplete(instance, this);

    refreshBindings(instance->context);

    QTimer::singleShot(RENDER_TIMEOUT, [=] {
        DesignerSupport::polishItems(m_view);

        for (QQuickItem* item : RenderUtils::allItems(instance)) {
            if (item)
                DesignerSupport::updateDirtyNode(item);
        }

        instance->preview = true;

        RenderResult result;
        result.uid = instance->uid;
        result.gui = instance->gui;
        result.popup = instance->popup;
        result.window = instance->window;
        result.visible = instance->visible;
        result.codeChanged = instance->codeChanged;
        result.geometryHash = instance->geometryHash;
        result.properties = RenderUtils::properties(instance);
        result.events = RenderUtils::events(instance);
        instance->codeChanged = false;
        result.errors = instance->errors;
        result.image = grabImage(instance, result.boundingRect);
        emit previewDone(result);

        auto ctx = instance->context;
        RenderUtils::deleteInstancesRecursive(instance, m_designerSupport);
        delete ctx;
    });
}

void RenderEngine::refreshAllBindings()
{
    DesignerSupport::refreshExpressions(m_view->rootContext());
    for (ControlInstance* formInstance : m_formInstances)
        DesignerSupport::refreshExpressions(formInstance->context);
    DesignerSupport::refreshExpressions(m_view->rootContext());
}

void RenderEngine::updateIndex(const QString& uid)
{
    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->parent);

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    repairIndexes(instance->parent);
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::updateId(const QString& uid, const QString& newId)
{
    Q_ASSERT(!newId.isEmpty());

    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->object);
    Q_ASSERT(instance->errors.isEmpty());

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    if (formInstance == instance)
        RenderUtils::setId(m_view->rootContext(), instance->object, instance->id, newId);
    RenderUtils::setId(instance->context, instance->object, instance->id, newId);

    instance->id = newId;

    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::createControl(const QString& dir, const QString& parentUid)
{
    ControlInstance* parentInstance = instanceForUid(parentUid);

    Q_ASSERT(parentInstance);

    ControlInstance* formInstance = formInstanceFor(parentInstance);

    Q_ASSERT(formInstance);

    ControlInstance* instance = createInstance(dir, parentInstance);

    RenderUtils::doComplete(instance, this);

    // No need to repairIndexes, since the instance is already added
    // as the last children on its parent
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::createForm(const QString& dir)
{
    ControlInstance* formInstance = createInstance(dir, nullptr);

    m_formInstances.append(formInstance);
    RenderUtils::doComplete(formInstance, this);

    // No need to repairIndexes, since form indexes ignored,
    // because they are put upon rootObject of the engine
    refreshAllBindings();
    scheduleRender(formInstance);
}

void RenderEngine::deleteForm(const QString& uid)
{
    ControlInstance* formInstance = instanceForUid(uid);

    Q_ASSERT(formInstance);

    m_formInstances.removeAll(formInstance);
    RenderUtils::cleanUpFormInstances(QList<ControlInstance*>{formInstance},
                                      m_view->rootContext(), m_designerSupport);

    // No need to repairIndexes, since form indexes ignored,
    // because they are put upon rootObject of the engine
    refreshAllBindings();

    for (ControlInstance* formInstance : m_formInstances)
        scheduleRender(formInstance);
}

void RenderEngine::deleteControl(const QString& uid)
{
    Q_ASSERT(!uid.isEmpty());
    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->parent);

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    RenderUtils::deleteInstancesRecursive(instance, m_designerSupport);
    instance->parent->children.removeAll(instance);

    repairIndexes(instance->parent);
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::refresh(const QString& formUid)
{
    if (formUid.isEmpty()) {
        for (ControlInstance* formInstance : m_formInstances) {
            RenderUtils::makeDirtyRecursive(formInstance);
            refreshBindings(formInstance->context);
            scheduleRender(formInstance);
        }
    } else {
        ControlInstance* formInstance = instanceForUid(formUid);

        Q_ASSERT(formInstance);

        RenderUtils::makeDirtyRecursive(formInstance);

        refreshBindings(formInstance->context);
        scheduleRender(formInstance);
    }
}

bool RenderEngine::hasInstanceForObject(const QObject* object) const
{
    Q_ASSERT(object);
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : RenderUtils::allSubInstance(formInstance)) {
            if (instance->object == object)
                return true;
            if (instance->window && RenderUtils::guiItem(instance) == object)
                return true;
            if (instance->popup && RenderUtils::guiItem(instance) == object)
                return true;
        }
    }
    return false;
}

bool RenderEngine::hasInstanceForUid(const QString& uid) const
{
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : RenderUtils::allSubInstance(formInstance)) {
            if (instance->uid == uid)
                return true;
        }
    }

    return false;
}

RenderEngine::ControlInstance* RenderEngine::instanceForObject(const QObject* object) const
{
    Q_ASSERT(object);
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : RenderUtils::allSubInstance(formInstance)) {
            if (instance->object == object)
                return instance;
            if (instance->window && RenderUtils::guiItem(instance) == object)
                return instance;
            if (instance->popup && RenderUtils::guiItem(instance) == object)
                return instance;
        }
    }
    return nullptr;
}

RenderEngine::ControlInstance* RenderEngine::instanceForUid(const QString& uid) const
{
    Q_ASSERT(!uid.isEmpty());
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* instance : RenderUtils::allSubInstance(formInstance)) {
            if (instance->uid == uid)
                return instance;
        }
    }

    return nullptr;
}

RenderEngine::ControlInstance* RenderEngine::formInstanceFor(const RenderEngine::ControlInstance* instance)
{
    for (ControlInstance* formInstance : m_formInstances) {
        for (ControlInstance* childInstance : RenderUtils::allSubInstance(formInstance)) {
            if (instance == childInstance)
                return formInstance;
        }
    }

    return nullptr;
}

RenderEngine::ControlInstance* RenderEngine::findNodeInstanceForItem(QQuickItem* item) const
{
    if (item) {
        if (hasInstanceForObject(item))
            return instanceForObject(item);
        else if (item->parentItem())
            return findNodeInstanceForItem(item->parentItem());
    }

    return nullptr;
}

void RenderEngine::scheduleRender(ControlInstance* formInstance)
{
    m_formInstanceSetForRender.insert(formInstance);
    if (!m_renderTimer->isActive())
        m_renderTimer->start();
}

void RenderEngine::scheduleRerenderForInvisibleInstances(RenderEngine::ControlInstance* formInstance)
{
    m_formInstanceSetForReRender.insert(formInstance);
    if (!m_reRenderTimer->isActive())
        m_reRenderTimer->start();
}

void RenderEngine::flushRenders()
{
    m_renderTimer->stop();
    for (ControlInstance* formInstance : m_formInstanceSetForRender) {
        if (!m_formInstances.contains(formInstance))
            continue; // Skip possible deleted forms

        DesignerSupport::polishItems(m_view);

        static DesignerSupport::DirtyType dirty = DesignerSupport::DirtyType(
                    DesignerSupport::TransformUpdateMask
                    | DesignerSupport::ContentUpdateMask
                    | DesignerSupport::Visible
                    | DesignerSupport::ZValue
                    | DesignerSupport::OpacityValue
                    | DesignerSupport::AllMask);

        for (QQuickItem* item : RenderUtils::allItems(formInstance)) {
            if (item) {
                if (hasInstanceForObject(item)) {
                    if (DesignerSupport::isDirty(item, dirty))
                        m_dirtyInstanceSet.insert(instanceForObject(item));
                } else if (DesignerSupport::isDirty(item, dirty)) {
                    if (ControlInstance* ancestorInstance = findNodeInstanceForItem(item->parentItem()))
                        m_dirtyInstanceSet.insert(ancestorInstance);
                }
                DesignerSupport::updateDirtyNode(item);
            }
        }

        if (!m_dirtyInstanceSet.isEmpty()) {
            emit renderDone(renderDirtyInstances(m_dirtyInstanceSet));
            if (m_initialized)
                scheduleRerenderForInvisibleInstances(formInstance);
            m_dirtyInstanceSet.clear();
        }

        RenderUtils::resetAllItems(formInstance);

        if (!m_initialized) {
            static QList<ControlInstance*> initializedInstances;
            initializedInstances.append(formInstance);
            if (initializedInstances.size() == m_formInstances.size()) {
                emit initializationProgressChanged(g_progress_4);
                m_initialized = true;
                for (ControlInstance* formInstance : m_formInstances)
                    scheduleRerenderForInvisibleInstances(formInstance);
            }
        }
    }
    m_formInstanceSetForRender.clear();
}

void RenderEngine::flushReRenders()
{
    m_reRenderTimer->stop();
    for (ControlInstance* formInstance : m_formInstanceSetForReRender) {
        if (!m_formInstances.contains(formInstance))
            continue; // Skip possible deleted forms

        OnlyOneInstanceList<ControlInstance*> rerenderDirtyInstanceSet;
        for (ControlInstance* instance : RenderUtils::allSubInstance(formInstance)) {
            if (!instance->errors.isEmpty())
                continue;

            if (!instance->gui)
                continue;

            if (!instance->needsRerender)
                continue;

            QQuickItem* item = RenderUtils::guiItem(instance);

            DesignerSupport::addDirty(item, DesignerSupport::AllMask);

            rerenderDirtyInstanceSet.insert(instance);
        }

        refreshBindings(formInstance->context);
        emit renderDone(renderDirtyInstances(rerenderDirtyInstanceSet));

        for (ControlInstance* rerenderInstance : rerenderDirtyInstanceSet)
            rerenderInstance->needsRerender = false;
    }
    m_formInstanceSetForReRender.clear();
}

QList<RenderResult> RenderEngine::renderDirtyInstances(const QList<RenderEngine::ControlInstance*>& instances)
{
    QList<RenderResult> results;
    for (int i = 0; i < instances.size(); ++i) {
        ControlInstance* instance = instances.at(i);
        Q_ASSERT(instance);
        RenderResult result;
        result.uid = instance->uid;
        result.gui = instance->gui;
        result.popup = instance->popup;
        result.window = instance->window;
        result.visible = instance->visible;
        result.codeChanged = instance->codeChanged;
        result.geometryHash = instance->geometryHash;
        result.properties = RenderUtils::properties(instance);
        result.events = RenderUtils::events(instance);
        instance->codeChanged = false;
        result.errors = instance->errors;
        result.image = grabImage(instance, result.boundingRect);
        results.append(result);

        if (instance->errors.isEmpty() && instance->gui)
            instance->needsRerender = PaintUtils::isBlankImage(result.image);

        if (!m_initialized) {
            g_progress += g_progressPerInstance;
            if (g_progress <= g_progress_4)
                emit initializationProgressChanged(g_progress);
        }
    }

    return results;
}

QRectF RenderEngine::boundingRectWithStepChilds(QQuickItem* item)
{
    QRectF boundingRect = item->clipRect();

    foreach (QQuickItem *childItem, item->childItems()) {
        if (!hasInstanceForObject(childItem)) {
            QRectF transformedRect = childItem->mapRectToItem(item, boundingRectWithStepChilds(childItem));
            if (RenderUtils::isRectangleSane(transformedRect))
                boundingRect = boundingRect.united(transformedRect);
        }
    }

    return boundingRect;
}

QRectF RenderEngine::boundingRect(QQuickItem* item)
{
    if (item) {
        if (item->clip())
            return item->boundingRect();
        else
            return boundingRectWithStepChilds(item);
    }
    return QRectF();
}

QImage RenderEngine::grabImage(const RenderEngine::ControlInstance* instance, QRectF& boundingRect)
{
    Q_ASSERT(instance);
    if (!instance->errors.isEmpty()) {
        return QImage();
    } else if (!instance->gui) {
        return QImage();
    } else {
        QColor winColor;
        if (instance->window)
            winColor = static_cast<QQuickWindow*>(instance->object)->color();
        if (instance->visible)
            return renderItem(RenderUtils::guiItem(instance->object), boundingRect, instance->preview, winColor);
        return QImage();
    }
}

QImage RenderEngine::renderItem(QQuickItem* item, QRectF& boundingRect, bool preview, const QColor& bgColor)
{
    Q_ASSERT(item);
    RenderUtils::updateDirtyNodesRecursive(item, this);

    boundingRect = preview ? QRectF(QPointF(), item->size()) : this->boundingRect(item);
    QSize size = boundingRect.size().toSize();
    size *= devicePixelRatio();

    QImage renderImage(size, QImage::Format_ARGB32_Premultiplied);
    renderImage.setDevicePixelRatio(devicePixelRatio());

    if (bgColor.isValid())
        renderImage.fill(bgColor);
    else
        renderImage.fill(Qt::transparent);

    QPainter painter(&renderImage);
    painter.drawImage(QRect({0, 0}, size / devicePixelRatio()),
                      m_designerSupport.renderImageForItem(item, boundingRect, size),
                      QRect({0, 0}, size));

    return renderImage;
}

void RenderEngine::refreshBindings(QQmlContext* context)
{
    m_designerSupport.refreshExpressions(context);
    m_designerSupport.refreshExpressions(m_view->rootContext());
}

void RenderEngine::repairIndexes(ControlInstance* parentInstance)
{
    Q_ASSERT(parentInstance);

    if (!parentInstance->gui)
        return;

    if (!parentInstance->errors.isEmpty())
        return;

    if (!parentInstance->object)
        return;

    if (parentInstance->children.isEmpty())
        return;

    std::sort(parentInstance->children.begin(), parentInstance->children.end(),
              [] (const ControlInstance* left, const ControlInstance* right) {
        return SaveUtils::controlIndex(left->dir) < SaveUtils::controlIndex(right->dir);
    });

    QQmlProperty defaultProperty(parentInstance->object);
    Q_ASSERT(defaultProperty.isValid());
    QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
    Q_ASSERT(childList.canClear());
    childList.clear();

    for (ControlInstance* childInstance : parentInstance->children) {
        if (childInstance->errors.isEmpty() && childInstance->object) {
            childList.append(childInstance->object);
            if (childInstance->window || childInstance->popup) { // We still reparent it anyway, may a window comes
                QQuickItem* item = RenderUtils::guiItem(childInstance->object);
                item->setParentItem(RenderUtils::guiItem(parentInstance->object));
            }
        }
    }
}

RenderEngine::ControlInstance* RenderEngine::createInstance(const QString& url)
{
    ComponentCompleteDisabler disabler;
    Q_UNUSED(disabler)

    auto instance = new RenderEngine::ControlInstance;
    instance->context = new QQmlContext(m_view->engine());

    QQmlComponent component(m_view->engine());
    component.loadUrl(QUrl::fromUserInput(url));

    QObject* object = component.beginCreate(instance->context);

    if (component.isError()) {
        if (object)
            delete object;

        instance->gui = false;
        instance->popup = false;
        instance->layout = false;
        instance->window = false;
        instance->object = nullptr;
        for (const QQmlError& e : component.errors())
            instance->errors.append(e);
        return instance;
    }

    Q_ASSERT(object);
    Q_ASSERT(!object->isWindowType() || object->inherits("QQuickWindow"));

    // Hides windows anyway, since we only need their contentItem to be visible
    RenderUtils::tweakObjects(object);
    // FIXME: what if the component is a Component qml type or crashing type? and other possibilities
    component.completeCreate();

    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    QQmlEnginePrivate::get(m_view->engine())->cache(object->metaObject());

    instance->object = object;
    instance->popup = object->inherits("QQuickPopup");
    instance->window = object->isWindowType();
    instance->layout = QQuickDesignerSupportMetaInfo::isSubclassOf(instance->object, "QQuickLayout");
    instance->gui = instance->window || instance->popup || object->inherits("QQuickItem");
    instance->visible = RenderUtils::isVisible(instance);

    QQmlProperty defaultProperty(m_view->rootObject());
    Q_ASSERT(defaultProperty.isValid());

    if (instance->gui) {
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        item->setFlag(QQuickItem::ItemHasContents, true);
        static_cast<QQmlParserStatus*>(item)->classBegin();
    }

    QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
    childList.append(instance->object);

    if (instance->gui) {
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        if (instance->window || instance->popup)
            item->setParentItem(RenderUtils::guiItem(m_view->rootObject())); // We still reparent it anyway, may a window comes
        m_designerSupport.refFromEffectItem(item);
        item->update();
    }

    // Make sure everything is visible
    if (instance->gui) {
        if (!instance->window)
            QQmlProperty(object, "visible", instance->context).write(true);
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        item->setVisible(true); // Especially important for popup
        item->update();
    }

    return instance;
}

RenderEngine::ControlInstance* RenderEngine::createInstance(const QString& dir,
                                                            ControlInstance* parentInstance,
                                                            QQmlContext* oldFormContext)
{
    Q_ASSERT_X(SaveUtils::isControlValid(dir), "createInstance", "Owctrl™ structure is corrupted.");

    ComponentCompleteDisabler disabler;
    Q_UNUSED(disabler)

    const QString& url = SaveUtils::toControlMainQmlFile(dir);

    auto instance = new RenderEngine::ControlInstance;
    instance->dir = dir;
    instance->id = SaveUtils::controlId(dir);
    instance->uid = SaveUtils::controlUid(dir);
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
        instance->layout = false;
        instance->popup = false;
        instance->window = false;
        instance->object = nullptr;
        for (const QQmlError& e : component.errors())
            instance->errors.append(e);

        m_dirtyInstanceSet.insert(instance);
        return instance;
    }

    Q_ASSERT(object);
    Q_ASSERT(!object->isWindowType() || object->inherits("QQuickWindow"));

    // Hides windows anyway, since we only need their contentItem to be visible
    RenderUtils::tweakObjects(object);
    // FIXME: what if the component is a Component qml type or crashing type? and other possibilities
    component.completeCreate();

    if (QQmlEngine::contextForObject(object) == nullptr)
        QQmlEngine::setContextForObject(object, instance->context);

    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    QQmlEnginePrivate::get(m_view->engine())->cache(object->metaObject());
    if (SaveUtils::isForm(dir))
        RenderUtils::setId(m_view->rootContext(), object, QString(), instance->id);
    RenderUtils::setId(instance->context, object, QString(), instance->id);

    instance->object = object;
    instance->layout = QQuickDesignerSupportMetaInfo::isSubclassOf(instance->object, "QQuickLayout");
    instance->popup = object->inherits("QQuickPopup");
    instance->window = object->isWindowType();
    instance->gui = instance->window || instance->popup || object->inherits("QQuickItem");
    instance->visible = RenderUtils::isVisible(instance);

    Q_ASSERT(!SaveUtils::isForm(dir) || instance->gui);

    if (!instance->gui)
        m_dirtyInstanceSet.insert(instance);

    if (SaveUtils::isForm(dir)) {
        QQuickItem* formItem = RenderUtils::guiItem(instance);
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
            QQuickItem* item = RenderUtils::guiItem(instance->object);
            item->setFlag(QQuickItem::ItemHasContents, true);
            static_cast<QQmlParserStatus*>(item)->classBegin();
        }

        QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
        childList.append(instance->object);

        if (instance->gui) {
            QQuickItem* item = RenderUtils::guiItem(instance->object);
            if (instance->window || instance->popup)
                item->setParentItem(RenderUtils::guiItem(parentObject)); // We still reparent it anyway, may a window comes
            m_designerSupport.refFromEffectItem(item);
            item->update();
        }
    }

    // Make sure everything is visible
    if (instance->gui) {
        if (!instance->window)
            QQmlProperty(object, "visible", instance->context).write(true);
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        item->setVisible(true); // Especially important for popup
        item->update();
    }

    return instance;
}

qreal RenderEngine::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

void RenderEngine::setDevicePixelRatio(qreal devicePixelRatio)
{
    if (m_devicePixelRatio != devicePixelRatio) {
        m_devicePixelRatio = devicePixelRatio;
        refresh(QString());
    }
}
