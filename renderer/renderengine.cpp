#include <renderengine.h>
#include <commandlineparser.h>
#include <saveutils.h>
#include <renderutils.h>
#include <renderinfo.h>
#include <utilityfunctions.h>
#include <components.h>
#include <paintutils.h>
#include <private/qquickdesignersupportmetainfo_p.h>
#include <private/qquickdesignersupportproperties_p.h>
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
    RENDER_TIMEOUT = 40,
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

void RenderEngine::init(const InitInfo& initInfo)
{
    if (m_initialized) {
        qWarning("RenderEngine: Re-initialization request rejected");
        return;
    }

    emit initializationProgressChanged(g_progress_1);

    /* Create instances, handle parent-child relationship, set ids, save form instances */
    QMap<QString, ControlInstance*> instanceTree;
    for (const QPair<QString, QString>& formPair : qAsConst(initInfo.forms)) {
        const QString& formPath = formPair.first;
        const QString& module = formPair.second;
        ControlInstance* formInstance = createInstance(formPath, module, nullptr);
        Q_ASSERT(formInstance);

        m_formInstances.append(formInstance);
        instanceTree.insert(formPath, formInstance);

        const QVector<QPair<QString, QString>>& children = initInfo.children.value(formPath);
        for (const QPair<QString, QString>& childPair : children) {
            const QString& childPath = childPair.first;
            const QString& mmodule = childPair.second;
            ControlInstance* parentInstance = instanceTree.value(SaveUtils::toDoubleUp(childPath));
            Q_ASSERT(parentInstance);
            ControlInstance* childInstance = createInstance(childPath, mmodule, parentInstance);
            Q_ASSERT(childInstance);
            instanceTree.insert(childPath, childInstance);
        }
    }

    emit initializationProgressChanged(g_progress_2);

    // FIXME: Object completetion order must be parent -> to -> child
    // according to Qt's internal completion order read here:
    // stackoverflow.com/questions/46196831
    for (ControlInstance* instance : qAsConst(instanceTree))
        RenderUtils::doComplete(instance, this);

    for (ControlInstance* instance : qAsConst(instanceTree)) {
        if (QQuickItem* item = RenderUtils::guiItem(instance))
            DesignerSupport::addDirty(item, DesignerSupport::AllMask);
    }

    /* Refresh expressions */
    refreshAllBindings();

    emit initializationProgressChanged(g_progress_3);

    /* Render */
    int totalInstanceCount = 0;
    for (RenderEngine::ControlInstance* formInstance : qAsConst(m_formInstances))
        totalInstanceCount += RenderUtils::countAllSubInstance(formInstance);
    g_progressPerInstance = qreal(g_progress_4 - g_progress_3) / totalInstanceCount;
    g_progress = g_progress_3;
    for (ControlInstance* formInstance : qAsConst(m_formInstances))
        scheduleRender(formInstance);
}

void RenderEngine::updateBinding(const QString& uid, const QString& bindingName, const QString& expression)
{
    ControlInstance* instance = instanceForUid(uid);

    Q_ASSERT(instance);
    Q_ASSERT(instance->object);
    Q_ASSERT(instance->errors.isEmpty());

    ControlInstance* formInstance = formInstanceFor(instance);

    Q_ASSERT(formInstance);

    if (expression.isEmpty()) {
        if (m_formInstances.contains(instance))
            updateFormCode(uid, instance->module);
        else
            updateControlCode(uid, instance->module);
    } else {
        RenderUtils::setInstancePropertyBinding(instance, bindingName, expression);
    }

    if (instance->parent && instance->parent->object)
        RenderUtils::refreshLayoutable(instance->parent);

    refreshBindings(formInstance->context);
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
    if (instance->parent && instance->parent->object)
        RenderUtils::refreshLayoutable(instance->parent);

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
    previousParentInstance->children.remove(instance);
    parentInstance->children.insert(instance);

    if (!instance->errors.isEmpty())
        return;

    RenderUtils::setInstanceParent(instance, RenderUtils::parentObject(parentInstance, m_view));
    RenderUtils::refreshLayoutable(previousParentInstance);
    RenderUtils::refreshLayoutable(parentInstance);

    repairIndexes(parentInstance);
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::updateControlCode(const QString& uid, const QString& module)
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

    ControlInstance* instance = createInstance(oldInstance->dir, module, oldInstance->parent);
    oldInstance->gui = instance->gui;
    oldInstance->module = instance->module;
    oldInstance->layout = instance->layout;
    oldInstance->popup = instance->popup;
    oldInstance->window = instance->window;
    oldInstance->visible = instance->visible;
    oldInstance->codeChanged = instance->codeChanged;
    oldInstance->geometrySyncKey = instance->geometrySyncKey;
    oldInstance->id = instance->id;
    oldInstance->object = instance->object;
    oldInstance->errors = instance->errors;
    oldInstance->parent->children.remove(instance);
    if (!instance->gui) { // Error condition is also included
        m_dirtyInstanceSet.remove(instance);
        m_dirtyInstanceSet.insert(oldInstance);
    }
    delete instance;

    std::sort(oldInstance->children.begin(), oldInstance->children.end(),
              [] (const ControlInstance* left, const ControlInstance* right) {
        return SaveUtils::controlIndex(left->dir) < SaveUtils::controlIndex(right->dir);
    });

    for (ControlInstance* childInstance : qAsConst(oldInstance->children)) {
        if (!childInstance->errors.isEmpty())
            continue;
        RenderUtils::setInstanceParent(childInstance, RenderUtils::parentObject(oldInstance, m_view));
    }

    // We delete previous instance object after we reparent all of its children into the new instance
    if (oldObject) {
        if (item) {
            item->setParentItem(nullptr);
            delete item;
        }
        if (oldObject != item)
            delete oldObject;
    }

    RenderUtils::doComplete(oldInstance, this);

    repairIndexes(oldInstance->parent);
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::updateFormCode(const QString& uid, const QString& module)
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

    ControlInstance* instance = createInstance(oldFormInstance->dir, module, nullptr, oldFormInstance->context);
    oldFormInstance->gui = instance->gui;
    oldFormInstance->module = instance->module;
    oldFormInstance->layout = instance->layout;
    oldFormInstance->popup = instance->popup;
    oldFormInstance->window = instance->window;
    oldFormInstance->visible = instance->visible;
    oldFormInstance->codeChanged = instance->codeChanged;
    oldFormInstance->geometrySyncKey = instance->geometrySyncKey;
    oldFormInstance->id = instance->id;
    oldFormInstance->object = instance->object;
    oldFormInstance->errors = instance->errors;
    if (!instance->gui) { // Error condition is also included
        m_dirtyInstanceSet.remove(instance);
        m_dirtyInstanceSet.insert(oldFormInstance);
    }
    delete instance;

    Q_ASSERT(!oldFormInstance->errors.isEmpty() || oldFormInstance->gui);

    std::sort(oldFormInstance->children.begin(), oldFormInstance->children.end(),
              [] (const ControlInstance* left, const ControlInstance* right) {
        return SaveUtils::controlIndex(left->dir) < SaveUtils::controlIndex(right->dir);
    });

    for (ControlInstance* childInstance : qAsConst(oldFormInstance->children)) {
        if (!childInstance->errors.isEmpty())
            continue;
        RenderUtils::setInstanceParent(childInstance, RenderUtils::parentObject(oldFormInstance, m_view));
    }

    // We delete previous instance object after we reparent all of its children into the new instance
    if (oldObject) {
        if (item) {
            item->setParentItem(nullptr);
            delete item;
        }
        if (oldObject != item)
            delete oldObject;
    }

    RenderUtils::doComplete(oldFormInstance, this);

    // NOTE: We would normally wait until render() function to detect if there are any dirts
    // But since we have a "margin" problem, all the (first) sub-childs should be updated in the first place
    // In order to let chilren to know if there were any "header", "footer" (therefore margin) changes.
    m_dirtyInstanceSet.insert(oldFormInstance);
    for (ControlInstance* instance : qAsConst(oldFormInstance->children))
        m_dirtyInstanceSet.insert(instance);

    // Form indexes ignored, since they are put upon rootObject of the engine
    // So, no need this: repairIndexes(oldFormInstance->parent??);
    refreshAllBindings();
    scheduleRender(oldFormInstance);
}

void RenderEngine::preview(const QString& url, const QString& module)
{
    Q_ASSERT(!url.isEmpty());

    ControlInstance* instance = createInstance(url, module);

    RenderUtils::doComplete(instance, this);

    refreshBindings(instance->context);

    QTimer::singleShot(RENDER_TIMEOUT, [=] {
        DesignerSupport::polishItems(m_view);

        const QList<QQuickItem*>& ais = RenderUtils::allItems(instance);
        for (QQuickItem* item : ais) {
            if (item)
                DesignerSupport::updateDirtyNode(item);
        }

        instance->preview = true;

        RenderInfo info;
        info.uid = instance->uid;
        info.gui = instance->gui;
        info.popup = instance->popup;
        info.overlayPopup = RenderUtils::isOverlayPopup(instance);
        info.window = instance->window;
        info.visible = instance->visible;
        info.codeChanged = instance->codeChanged;
        info.geometrySyncKey = instance->geometrySyncKey;
        // info.margins = RenderUtils::margins(instance);
        // info.properties = RenderUtils::properties(instance);
        // info.events = RenderUtils::events(instance);
        // info.anchors = RenderUtils::anchors(instance, this);
        instance->codeChanged = false;
        info.errors = instance->errors;
        info.image = grabImage(instance, info.surroundingRect);
        emit previewDone(info);

        auto ctx = instance->context;
        RenderUtils::deleteInstancesRecursive(instance, m_designerSupport);
        delete ctx;
    });
}

void RenderEngine::refreshAllBindings()
{
    DesignerSupport::refreshExpressions(m_view->rootContext());
    for (ControlInstance* formInstance : qAsConst(m_formInstances))
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

void RenderEngine::createControl(const QString& dir, const QString& module, const QString& parentUid)
{
    ControlInstance* parentInstance = instanceForUid(parentUid);

    Q_ASSERT(parentInstance);

    ControlInstance* formInstance = formInstanceFor(parentInstance);

    Q_ASSERT(formInstance);

    ControlInstance* instance = createInstance(dir, module, parentInstance);

    RenderUtils::doComplete(instance, this);

    // No need to repairIndexes, since the instance is already added
    // as the last children on its parent
    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::createForm(const QString& dir, const QString& module)
{
    ControlInstance* formInstance = createInstance(dir, module, nullptr);

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

    for (ControlInstance* formInstance : qAsConst(m_formInstances))
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
    instance->parent->children.remove(instance);

    refreshBindings(formInstance->context);
    scheduleRender(formInstance);
}

void RenderEngine::refresh(const QString& formUid)
{
    if (formUid.isEmpty()) {
        for (ControlInstance* formInstance : qAsConst(m_formInstances)) {
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
    for (ControlInstance* formInstance : qAsConst(m_formInstances)) {
        const QList<ControlInstance*>& asis = RenderUtils::allSubInstance(formInstance);
        for (ControlInstance* instance : asis) {
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
    for (ControlInstance* formInstance : qAsConst(m_formInstances)) {
        const QList<ControlInstance*>& asis = RenderUtils::allSubInstance(formInstance);
        for (ControlInstance* instance : asis) {
            if (instance->uid == uid)
                return true;
        }
    }
    return false;
}

RenderEngine::ControlInstance* RenderEngine::instanceForObject(const QObject* object) const
{
    if (object == 0)
        return nullptr;
    for (ControlInstance* formInstance : qAsConst(m_formInstances)) {
        const QList<ControlInstance*>& asis = RenderUtils::allSubInstance(formInstance);
        for (ControlInstance* instance : asis) {
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
    for (ControlInstance* formInstance : qAsConst(m_formInstances)) {
        const QList<ControlInstance*>& asis = RenderUtils::allSubInstance(formInstance);
        for (ControlInstance* instance : asis) {
            if (instance->uid == uid)
                return instance;
        }
    }

    return nullptr;
}

RenderEngine::ControlInstance* RenderEngine::formInstanceFor(const RenderEngine::ControlInstance* instance)
{
    for (ControlInstance* formInstance : qAsConst(m_formInstances)) {
        const QList<ControlInstance*>& asis = RenderUtils::allSubInstance(formInstance);
        for (ControlInstance* childInstance : asis) {
            if (instance == childInstance)
                return formInstance;
        }
    }

    return nullptr;
}

RenderEngine::ControlInstance* RenderEngine::findNodeInstanceForItem(const QQuickItem* item) const
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

    DesignerSupport::polishItems(m_view);

    for (ControlInstance* formInstance : qAsConst(m_formInstanceSetForRender)) {
        if (!m_formInstances.contains(formInstance))
            continue; // Skip possible deleted forms

        static DesignerSupport::DirtyType dirty = DesignerSupport::DirtyType(
                    DesignerSupport::TransformUpdateMask
                    | DesignerSupport::ContentUpdateMask
                    | DesignerSupport::Visible
                    | DesignerSupport::ZValue
                    | DesignerSupport::OpacityValue
                    | DesignerSupport::AllMask);

        const QList<QQuickItem*>& ais = RenderUtils::allItems(formInstance);
        for (QQuickItem* item : ais) {
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
                for (ControlInstance* formInstance : qAsConst(m_formInstances))
                    scheduleRerenderForInvisibleInstances(formInstance);
            }
        }
    }
    m_formInstanceSetForRender.clear();
}

void RenderEngine::flushReRenders()
{
    m_reRenderTimer->stop();
    for (ControlInstance* formInstance : qAsConst(m_formInstanceSetForReRender)) {
        if (!m_formInstances.contains(formInstance))
            continue; // Skip possible deleted forms

        OnlyOneInstanceList<ControlInstance*> rerenderDirtyInstanceSet;
        const QList<ControlInstance*>& asis = RenderUtils::allSubInstance(formInstance);
        for (ControlInstance* instance : asis) {
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

        for (ControlInstance* rerenderInstance : qAsConst(rerenderDirtyInstanceSet))
            rerenderInstance->needsRerender = false;
    }
    m_formInstanceSetForReRender.clear();
}

QList<RenderInfo> RenderEngine::renderDirtyInstances(const QList<RenderEngine::ControlInstance*>& instances)
{
    QList<RenderInfo> infos;
    for (int i = 0; i < instances.size(); ++i) {
        ControlInstance* instance = instances.at(i);
        Q_ASSERT(instance);
        RenderInfo info;
        info.uid = instance->uid;
        info.gui = instance->gui;
        info.popup = instance->popup;
        info.overlayPopup = RenderUtils::isOverlayPopup(instance);
        info.window = instance->window;
        info.visible = instance->visible;
        info.codeChanged = instance->codeChanged;
        info.margins = RenderUtils::margins(instance);
        info.geometrySyncKey = instance->geometrySyncKey;
        info.properties = RenderUtils::properties(instance);
        info.events = RenderUtils::events(instance);
        instance->codeChanged = false;
        info.errors = instance->errors;
        info.anchors = RenderUtils::anchors(instance, this);
        info.image = grabImage(instance, info.surroundingRect);
        infos.append(info);

        if (instance->errors.isEmpty() && instance->gui)
            instance->needsRerender = PaintUtils::isBlankImage(info.image);

        if (!m_initialized) {
            g_progress += g_progressPerInstance;
            if (g_progress <= g_progress_4)
                emit initializationProgressChanged(g_progress);
        }
    }

    return infos;
}

QRectF RenderEngine::boundingRectWithStepChilds(QQuickItem* item)
{
    QRectF boundingRect = item->clipRect();

    const QList<QQuickItem*>& childs = item->childItems();
    for (QQuickItem *childItem : childs) {
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
    const QSize& size = (boundingRect.size() * devicePixelRatio()).toSize();

    if (bgColor.isValid()) {
        QImage original = m_designerSupport.renderImageForItem(item, boundingRect, size);
        original.setDevicePixelRatio(devicePixelRatio());
        if (original.isNull())
            return QImage();
        QImage modified(original.size(), QImage::Format_ARGB32_Premultiplied);
        modified.setDevicePixelRatio(devicePixelRatio());
        QPainter painter(&modified);
        painter.translate(-boundingRect.topLeft());
        painter.fillRect(QRectF(QPointF(), item->size()), bgColor);
        painter.translate(boundingRect.topLeft());
        painter.drawImage(QRectF(QPointF(), QSizeF(original.size()) / devicePixelRatio()),
                          original, original.rect());
        return modified;
    } else {
        return m_designerSupport.renderImageForItem(item, boundingRect, size);
    }
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

    for (ControlInstance* childInstance : qAsConst(parentInstance->children)) {
        if (parentInstance->object && childInstance->object)
            RenderUtils::setInstanceParent(childInstance, parentInstance->object);
    }
}

RenderEngine::ControlInstance* RenderEngine::createInstance(const QString& url, const QString& module)
{
    ComponentCompleteDisabler disabler;
    Q_UNUSED(disabler)

    auto instance = new RenderEngine::ControlInstance;
    instance->context = new QQmlContext(m_view->engine());

    QQmlComponent component(m_view->engine());
    component.loadUrl(QUrl::fromUserInput(RenderUtils::mockUrl(url, module)));

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

    if (instance->gui) {
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        item->setFlag(QQuickItem::ItemHasContents, true);
        if (auto qml = dynamic_cast<QQmlParserStatus*>(instance->object))
            qml->classBegin();
    }

    RenderUtils::setInstanceParent(instance, m_view->rootObject());

    if (instance->gui) {
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        Q_ASSERT(item->window());
        m_designerSupport.refFromEffectItem(item);
        item->update();
    }

    // Make sure everything is visible
    if (instance->gui) {
        if (!instance->window)
            QQmlProperty::write(object, "visible", true, instance->context);
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        item->setVisible(true); // Especially important for popup
        item->update();
    }

    return instance;
}

RenderEngine::ControlInstance* RenderEngine::createInstance(const QString& dir, const QString& module,
                                                            ControlInstance* parentInstance,
                                                            QQmlContext* oldFormContext)
{
    Q_ASSERT_X(SaveUtils::isControlValid(dir), "createInstance", "Owctrl™ structure is corrupted.");

    const QString& url = SaveUtils::toControlMainQmlFile(dir);

    auto instance = new RenderEngine::ControlInstance;
    instance->module = module;
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
        parentInstance->children.insert(instance);
        instance->parent = parentInstance;
        instance->context = parentInstance->context;
    }

    m_view->engine()->clearComponentCache();
    QQmlComponent component(m_view->engine());
    component.loadUrl(QUrl::fromLocalFile(RenderUtils::mockUrl(url, module)));

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
    Q_ASSERT(instance->errors.isEmpty());
    Q_ASSERT(!object->isWindowType() || object->inherits("QQuickWindow"));

    // Hides windows anyway, since we only need their contentItem to be visible
    // FIXME: what if the component is a Component qml type or crashing type? and other possibilities
    ComponentCompleteDisabler disabler;
    Q_UNUSED(disabler)
    RenderUtils::tweakObjects(object);
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
        if (auto qml = dynamic_cast<QQmlParserStatus*>(instance->object))
            qml->classBegin();
        formItem->setParentItem(m_view->rootObject());
        Q_ASSERT(formItem->window());
        m_designerSupport.refFromEffectItem(formItem);
        formItem->update();
    } else {
        if (instance->gui) {
            QQuickItem* item = RenderUtils::guiItem(instance->object);
            item->setFlag(QQuickItem::ItemHasContents, true);
            if (auto qml = dynamic_cast<QQmlParserStatus*>(instance->object))
                qml->classBegin();
        }

        RenderUtils::setInstanceParent(instance, RenderUtils::parentObject(parentInstance, m_view));

        if (instance->gui) {
            QQuickItem* item = RenderUtils::guiItem(instance->object);
            Q_ASSERT(item->window());
            m_designerSupport.refFromEffectItem(item);
            item->update();
        }
    }

    QQuickDesignerSupportProperties::registerCustomData(object); // Needed for setInstancePropertyBinding

    // Make sure everything is visible
    if (instance->gui) {
        if (!instance->window)
            QQmlProperty::write(object, "visible", true, instance->context);
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
