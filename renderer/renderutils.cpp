#include <renderutils.h>
#include <saveutils.h>
#include <renderinfo.h>
#include <utilityfunctions.h>
#include <parserutils.h>

#include <QAnimationDriver>
#include <QQuickView>
#include <QQmlContext>

#include <private/qqmltimer_p.h>
#include <private/qquicktext_p.h>
#include <private/qqmlengine_p.h>
#include <private/qquicktextedit_p.h>
#include <private/qquickanimation_p.h>
#include <private/qquicktextinput_p.h>
#include <private/qquicktransition_p.h>
#include <private/qquickdesignersupportproperties_p.h>
#include <private/qquickpopup_p.h>
#include <private/qqmlvme_p.h>
#include <private/qquickitem_p.h>
#include <private/qquickoverlay_p.h>

namespace {

void hideWindow(QObject* object)
{
    if (!object)
        return;

    if (object->isWindowType()) {
        auto window = static_cast<QWindow*>(object);
        window->setFlags(Qt::Tool
                         | Qt::X11BypassWindowManagerHint
                         | Qt::NoDropShadowWindowHint
                         | Qt::FramelessWindowHint
                         | Qt::WindowStaysOnBottomHint
                         | Qt::WindowTransparentForInput
                         | Qt::WindowDoesNotAcceptFocus);
        window->create();
        window->show();
        window->hide();
        window->setProperty("visible", false);
        window->setProperty("visibility", QWindow::Hidden);
    }
}

void stopAnimation(QObject *object)
{
    if (object == nullptr)
        return;

    QQuickTransition *transition = qobject_cast<QQuickTransition*>(object);
    QQuickAbstractAnimation *animation = qobject_cast<QQuickAbstractAnimation*>(object);
    QQmlTimer *timer = qobject_cast<QQmlTimer*>(object);
    QObject* animatedImage = object->inherits("QQuickAnimatedImage") ? object : nullptr;
    if (transition) {
        transition->setFromState("");
        transition->setToState("");
    } else if (animation) {
        //        QQuickScriptAction *scriptAimation = qobject_cast<QQuickScriptAction*>(animation);
        //        if (scriptAimation) FIXME
        //            scriptAimation->setScript(QQmlScriptString());
        animation->setLoops(1);
        animation->complete();
        animation->setDisableUserControl();
    } else if (timer) {
        timer->blockSignals(true);
    } else if (animatedImage) {
        animatedImage->setProperty("playing", false);
    }
}

void disableTextCursor(QQuickItem* item)
{
    for (QQuickItem* childItem : item->childItems())
        disableTextCursor(childItem);

    QQuickTextInput* textInput = qobject_cast<QQuickTextInput*>(item);
    if (textInput)
        textInput->setCursorVisible(false);

    QQuickTextEdit* textEdit = qobject_cast<QQuickTextEdit*>(item);
    if (textEdit)
        textEdit->setCursorVisible(false);
}

void allSubObject(QObject* object, QObjectList& objectList)
{
    // don't add null pointer and stop if the object is already in the list
    if (!object || objectList.contains(object))
        return;

    objectList.append(object);

    for (int index = QObject::staticMetaObject.propertyOffset();
         index < object->metaObject()->propertyCount();
         index++) {
        QMetaProperty metaProperty = object->metaObject()->property(index);

        // search recursive in property objects
        if (metaProperty.isReadable()
                && metaProperty.isWritable()
                && QQmlMetaType::isQObject(metaProperty.userType())) {
            if (strcmp(metaProperty.name(), "parent") != 0) {
                QObject *propertyObject = QQmlMetaType::toQObject(metaProperty.read(object));
                allSubObject(propertyObject, objectList);
            }
        }

        // search recursive in property object lists
        if (metaProperty.isReadable() && QQmlMetaType::isList(metaProperty.userType())) {
            QQmlListReference list(object, metaProperty.name());
            if (list.canCount() && list.canAt()) {
                for (int i = 0; i < list.count(); i++) {
                    QObject *propertyObject = list.at(i);
                    allSubObject(propertyObject, objectList);
                }
            }
        }
    }

    // search recursive in object children list
    for (QObject *childObject : object->children())
        allSubObject(childObject, objectList);

    // search recursive in quick item childItems list
    QQuickItem *quickItem = qobject_cast<QQuickItem*>(object);
    if (quickItem) {
        for (QQuickItem *childItem : quickItem->childItems()) {
            allSubObject(childItem, objectList);
        }
    }
}

QList<QQuickItem*> allChildItemsRecursive(const QQuickItem* parentItem)
{
    QList<QQuickItem*> itemList;
    if (!parentItem)
        return itemList;

    itemList.append(parentItem->childItems());

    for (QQuickItem* childItem : parentItem->childItems())
        itemList.append(allChildItemsRecursive(childItem));

    return itemList;
}

void doComponentCompleteRecursive(QObject* object, const RenderEngine* engine)
{
    if (object) {
        QQuickItem *item = qobject_cast<QQuickItem*>(object);

        if (item && DesignerSupport::isComponentComplete(item))
            return;

        DesignerSupport::emitComponentCompleteSignalForAttachedProperty(object);

        QList<QObject*> childList = object->children();
        // TODO: Do we need that? allSubObject(object, childList);

        if (item) {
            for (QQuickItem *childItem : item->childItems()) {
                if (!childList.contains(childItem))
                    childList.append(childItem);
            }
        }

        for (QObject *child : childList) {
            if (!engine->hasInstanceForObject(child))
                doComponentCompleteRecursive(child, engine);
        }

        if (item) {
            static_cast<QQmlParserStatus*>(item)->componentComplete();
        } else {
            QQmlParserStatus *qmlParserStatus = dynamic_cast<QQmlParserStatus*>(object);
            if (qmlParserStatus)
                qmlParserStatus->componentComplete();
        }
    }
}

bool metaPropertyPasses(const QMetaProperty& mp)
{
    auto type = mp.type();

    if (!mp.isValid())
        return false;

    if (QString(mp.typeName()).contains("*")
            || QString(mp.name()).contains(".")
            || QString(mp.name()).count(".") > 1
            || QString(mp.name()).startsWith("__")
            || mp.isFlagType()) {
        return false;
    }

    if (type != QVariant::BitArray
            && type != QVariant::Bitmap
            && type != QVariant::Bool
            && type != QVariant::Brush
            && type != QVariant::ByteArray
            && type != QVariant::Char
            && type != QVariant::Color
            && type != QVariant::Cursor
            && type != QVariant::Date
            && type != QVariant::DateTime
            && type != QVariant::Double
            && type != QVariant::EasingCurve
            && type != QVariant::Uuid
            && type != QVariant::ModelIndex
            && type != QVariant::PersistentModelIndex
            && type != QVariant::Font
            && type != QVariant::Hash
            && type != QVariant::Icon
            && type != QVariant::Image
            && type != QVariant::Int
            && type != QVariant::KeySequence
            && type != QVariant::Line
            && type != QVariant::LineF
            && type != QVariant::List
            && type != QVariant::Locale
            && type != QVariant::LongLong
            && type != QVariant::Map
            && type != QVariant::Matrix
            && type != QVariant::Transform
            && type != QVariant::Matrix4x4
            && type != QVariant::Palette
            && type != QVariant::Pen
            && type != QVariant::Pixmap
            && type != QVariant::Point
            && type != QVariant::PointF
            && type != QVariant::Polygon
            && type != QVariant::PolygonF
            && type != QVariant::Quaternion
            && type != QVariant::Rect
            && type != QVariant::RectF
            && type != QVariant::RegExp
            && type != QVariant::RegularExpression
            && type != QVariant::Region
            && type != QVariant::Size
            && type != QVariant::SizeF
            && type != QVariant::SizePolicy
            && type != QVariant::String
            && type != QVariant::StringList
            && type != QVariant::TextFormat
            && type != QVariant::TextLength
            && type != QVariant::Time
            && type != QVariant::UInt
            && type != QVariant::ULongLong
            && type != QVariant::Url
            && type != QVariant::Vector2D
            && type != QVariant::Vector3D
            && type != QVariant::Vector4D) {
        return false;
    }

    return true;
}

QString cleanClassName(const QMetaObject* metaObject)
{
    QString className = metaObject->className();
    className.replace("QQuickWindowQmlImpl", "Window");
    className.replace("QQuick", "");
    className.replace("QDeclarative", "");
    className.replace("QQml", "");
    className.replace("QWindow", "Window");
    className.replace(QRegularExpression("_QMLTYPE_\\d+"), "");
    className.replace(QRegularExpression("_QML_\\d+"), "");
    return className;
}

QString cleanScopeName(const QString& scope)
{
    QString sc(scope);
    sc.replace("QQuickWindowQmlImpl", "Window");
    sc.replace("QQuick", "");
    sc.replace("QDeclarative", "");
    sc.replace("QQml", "");
    sc.replace("QWindow", "Window");
    return sc;
}

bool propertyExistsInNodes(const QVector<PropertyNode>& nodes, const QString& property)
{
    for (const auto& node : nodes) {
        for (const auto& propertyName : node.properties.keys()) {
            if (propertyName == property)
                return true;
        }
        for (const auto& e : node.enums) {
            if (e.name == property)
                return true;
        }
    }
    return false;
}

bool unitePropertiesIfPossible(QVector<PropertyNode>& nodes, const QString& className,
                               const QMap<QString, QVariant>& properties, const QVector<Enum>& enums)
{
    for (auto& node : nodes) {
        if (node.cleanClassName == className) {
            node.properties.unite(properties);
            node.enums << enums;
            return true;
        }
    }

    return false;
}

QVector<Enum> subEnums(QMetaProperty property, const QObject* parentObject)
{
    QVector<Enum> enums;

    if (!property.isConstant() || !property.isReadable() || !property.isDesignable())
        return enums;

    auto object = qvariant_cast<QObject*>(property.read(parentObject));

    if (object) {
        auto metaObject = object->metaObject();

        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
            const auto& subproperty = metaObject->property(i);

            if (subproperty.isReadable()
                    && subproperty.isWritable()
                    && !QString(subproperty.typeName()).contains("*")
                    && metaPropertyPasses(subproperty)) {
                if (subproperty.isEnumType() && !subproperty.isFlagType()) {
                    auto metaEnum = subproperty.enumerator();

                    Enum e;
                    e.name = subproperty.name();
                    e.scope = cleanScopeName(metaEnum.scope());
                    e.value = metaEnum.valueToKey(subproperty.read(object).toInt());

                    for (int i = metaEnum.keyCount(); i--;) {
                        if (QString(metaEnum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
                            e.keys[metaEnum.key(i)] = metaEnum.value(i);
                    }

                    enums << e;
                }
            }
        }
    }

    return enums;
}

QMap<QString, QVariant> subProperties(QMetaProperty property, const QObject* parentObject)
{
    QMap<QString, QVariant> properties;

    if (QString::fromUtf8(property.name()) == "contentItem"
            || QString::fromUtf8(property.name()) == "parent") { // TODO: Improve this
        return properties;
    }

    if (!property.isConstant()
            || !property.isReadable()
            || !property.isDesignable()) {
        return properties;
    }

    auto object = qvariant_cast<QObject*>(property.read(parentObject));

    if (object) {
        auto metaObject = object->metaObject();

        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
            const auto& subproperty = metaObject->property(i);

            if (subproperty.isReadable()
                    && subproperty.isWritable()
                    && !QString(subproperty.typeName()).contains("*")
                    && metaPropertyPasses(subproperty)) {
                properties[property.name() + QString(".") + subproperty.name()] = subproperty.read(object);
            }
        }
    }

    return properties;
}
}

QList<QQuickItem*> RenderUtils::allItems(RenderEngine::ControlInstance* formInstance)
{
    Q_ASSERT(formInstance);
    QList<QQuickItem *> itemList;

    QQuickItem* item = guiItem(formInstance);
    if (item) {
        if (item->parentItem())
            itemList.append(item->parentItem());

        itemList.append(item);
        itemList.append(allChildItemsRecursive(item));
    }

    for (RenderEngine::ControlInstance* instance : RenderUtils::allSubInstance(formInstance)) {
        Q_ASSERT(instance);
        if (instance->gui) {
            QQuickItem* item = RenderUtils::guiItem(instance);
            Q_ASSERT(item);
            if (!itemList.contains(item)) {
                itemList.append(item);
                itemList.append(allChildItemsRecursive(item));
            }
        }
    }

    return itemList;
}

QVector<QString> RenderUtils::events(const RenderEngine::ControlInstance* instance)
{
    Q_ASSERT(instance);
    QVector<QString> events;
    if (!instance->errors.isEmpty())
        return events;

    QObject* object = instance->object;
    Q_ASSERT(object);

    auto metaObject = object->metaObject();

    for (int i = metaObject->methodCount(); i--;) {
        if (metaObject->method(i).methodType() == QMetaMethod::Signal
                && !metaObject->method(i).name().startsWith("__"))
            events << metaObject->method(i).name();
    }

    return events;
}

QList<RenderEngine::ControlInstance*> RenderUtils::allSubInstance(RenderEngine::ControlInstance* parentInstance)
{
    QList<RenderEngine::ControlInstance*> instances;
    if (!parentInstance)
        return instances;

    instances.append(parentInstance);

    for (RenderEngine::ControlInstance* childInstance : parentInstance->children)
        instances.append(allSubInstance(childInstance));

    return instances;
}

int RenderUtils::countAllSubInstance(const RenderEngine::ControlInstance* parentInstance)
{
    int counter = 0;

    if (!parentInstance)
        return counter;

    ++counter;

    for (RenderEngine::ControlInstance* childInstance : parentInstance->children)
        counter += countAllSubInstance(childInstance);

    return counter;
}

bool RenderUtils::isOverlayPopup(const RenderEngine::ControlInstance* instance)
{
    if (!instance->popup)
        return false;

    const auto centerIn = QQmlProperty::read(instance->object, "anchors.centerIn", instance->context).value<QQuickItem*>();

    if (centerIn == 0)
        return false;

    return centerIn->inherits("QQuickOverlay");
}

QVariant RenderUtils::evaluate(const RenderEngine::ControlInstance* instance, const QString& expression)
{
    QQmlExpression expr(instance->context, instance->object, expression);
    return expr.evaluate();
}

bool RenderUtils::isVisible(const RenderEngine::ControlInstance* instance)
{
    if (instance->gui) {
        bool visible;
        if (instance->window || instance->popup)
            visible = evaluate(instance, ParserUtils::property(instance->dir, "visible")).toBool();
        else
            visible = QQmlProperty::read(instance->object, "visible").toBool();
        if (instance->window)
            return visible && !ParserUtils::property(instance->dir, "visibility").contains("Hidden");
        return visible;
    }
    return false;
}

bool RenderUtils::isRectangleSane(const QRectF& rect)
{
    return rect.isValid() && (rect.width() < 10000) && (rect.height() < 10000);
}

void RenderUtils::setInstanceParent(RenderEngine::ControlInstance* instance, QObject* parentObject)
{
    Q_ASSERT(parentObject);
    Q_ASSERT(instance->object);

    if (instance->gui) {
        QQuickItem* item = guiItem(instance->object);
        if (item->parentItem())
            item->setParentItem(nullptr);
        //        if (instance->window || instance->popup) // We still reparent it anyway, may a window comes
        item->setParentItem(guiItem(parentObject));
    }

    instance->object->setParent(parentObject);

    QQmlProperty defaultProperty(parentObject);
    Q_ASSERT(defaultProperty.isValid());

    QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
    Q_ASSERT(!instance->gui || childList.canAppend());

    // Workaround against QQuickContainer's insertItem() bug
    if (childList.canAppend()) {
        const bool completeDisabled = !QQmlVME::componentCompleteEnabled();
        if (completeDisabled)
            QQmlVME::enableComponentComplete();
        childList.append(instance->object);
        if (completeDisabled)
            QQmlVME::disableComponentComplete();
    }

    Q_ASSERT(!instance->gui || guiItem(instance->object)->parentItem());
}

void RenderUtils::refreshLayoutable(const RenderEngine::ControlInstance* instance)
{
    if (instance->layout == false)
        return;
    if (QQuickItem* item = guiItem(instance->object))
        item->polish();
}

void RenderUtils::resetAllItems(RenderEngine::ControlInstance* formInstance)
{
    Q_ASSERT(formInstance);
    for (QQuickItem* item : allItems(formInstance))
        DesignerSupport::resetDirty(item);
}

void RenderUtils::doComplete(RenderEngine::ControlInstance* instance, const RenderEngine* engine)
{
    Q_ASSERT(instance);
    if (!instance->errors.isEmpty())
        return;

    Q_ASSERT(instance->object);

    doComponentCompleteRecursive(instance->object, engine);

    // FIXME: How do we handle header, footer or toolbar items of an ApplicationWindow?
    if (instance->gui) {
        QQuickItem* item = RenderUtils::guiItem(instance->object);
        Q_ASSERT(item);
        disableTextCursor(item);
        item->update();
    }
}

void RenderUtils::tweakObjects(QObject* object)
{
    if (!object)
        return;

    QObjectList objectList;
    allSubObject(object, objectList);
    for(QObject* childObject : objectList) {
        stopAnimation(childObject);
        hideWindow(childObject);
    }
}

void RenderUtils::updateDirtyNodesRecursive(QQuickItem* parentItem, RenderEngine* engine)
{
    Q_ASSERT(engine);
    Q_ASSERT(parentItem);

    for (QQuickItem *childItem : parentItem->childItems()) {
        if (!engine->hasInstanceForObject(childItem))
            updateDirtyNodesRecursive(childItem, engine);
    }

    RenderUtils::disableNativeTextRendering(parentItem);
    DesignerSupport::updateDirtyNode(parentItem);
}

void RenderUtils::disableNativeTextRendering(QQuickItem* item)
{
    QQuickText* text = qobject_cast<QQuickText*>(item);
    if (text)
        text->setRenderType(QQuickText::QtRendering);

    QQuickTextInput* textInput = qobject_cast<QQuickTextInput*>(item);
    if (textInput)
        textInput->setRenderType(QQuickTextInput::QtRendering);

    QQuickTextEdit* textEdit = qobject_cast<QQuickTextEdit*>(item);
    if (textEdit)
        textEdit->setRenderType(QQuickTextEdit::QtRendering);
}

void RenderUtils::stopUnifiedTimer()
{
    QUnifiedTimer::instance()->setSlowdownFactor(0.00001);
    QUnifiedTimer::instance()->setSlowModeEnabled(true);
}

void RenderUtils::makeDirtyRecursive(RenderEngine::ControlInstance* beginningInstance)
{
    QQuickItem* item = guiItem(beginningInstance->object);

    if (item)
        DesignerSupport::addDirty(item, DesignerSupport::AllMask);

    for (RenderEngine::ControlInstance* childInstance : beginningInstance->children)
        makeDirtyRecursive(childInstance);
}

void RenderUtils::setInstancePropertyBinding(const RenderEngine::ControlInstance* instance,
                                             const QString& bindingName, const QString& expression)
{
    Q_ASSERT(instance);
    Q_ASSERT(instance->errors.isEmpty());
    Q_ASSERT(instance->object);

    if (expression == "Overlay.overlay") {
        QObject* parent = instance->object->parent();
        while (parent) {
            if (parent->inherits("QQuickWindow"))
                break;
            parent = parent->parent();
        }
        if (auto window = qobject_cast<QQuickWindow*>(parent)) {
            QQmlProperty::write(instance->object, bindingName,
                                QVariant::fromValue(QQuickOverlay::overlay(window)),
                                instance->context);
        }
    } else {
        // WARNING: Don't use for visible, visibility, x, y, width, height properties,
        // or implement it like how setInstancePropertyVariant does implement them
        QQuickDesignerSupportProperties::setPropertyBinding(instance->object, instance->context,
                                                            bindingName.toUtf8(), expression);
    }

    // TODO: Remove this. This is a workaround for a bug that an
    // item persists on the bad position after fill -> undo-fill
    if (UtilityFunctions::anchorLineNames().contains(bindingName)) {
        if (QQuickItem* item = guiItem(instance->object)) {
            bool conversationSuccessful = true;
            qreal x = ParserUtils::property(instance->dir, "x").toDouble(&conversationSuccessful);
            if (!conversationSuccessful)
                x = item->x();
            qreal y = ParserUtils::property(instance->dir, "y").toDouble(&conversationSuccessful);
            if (!conversationSuccessful)
                y = item->y();
            qreal w = ParserUtils::property(instance->dir, "width").toDouble(&conversationSuccessful);
            if (!conversationSuccessful)
                w = item->implicitWidth();
            qreal h = ParserUtils::property(instance->dir, "height").toDouble(&conversationSuccessful);
            if (!conversationSuccessful)
                h = item->implicitHeight();
            item->setPosition(QPointF(x + 1, y + 1));
            item->setPosition(QPointF(x - 1, y - 1));
            item->setSize(QSizeF(w + 1, h + 1));
            item->setSize(QSizeF(w - 1, h - 1));
        }
    }

    DesignerSupport::addDirty(RenderUtils::guiItem(instance), DesignerSupport::ContentUpdateMask);
    if (instance->parent && instance->parent->object)
        DesignerSupport::addDirty(RenderUtils::guiItem(instance->parent), DesignerSupport::ChildrenUpdateMask);
}

void RenderUtils::setInstancePropertyVariant(RenderEngine::ControlInstance* instance,
                                             const QString& propertyName,
                                             const QVariant& propertyValue)
{
    Q_ASSERT(instance);
    Q_ASSERT(instance->errors.isEmpty());
    Q_ASSERT(instance->object);

    QQmlProperty property(instance->object, propertyName, instance->context);

    if (!property.isValid())
        return;

    if (propertyName == "visible" || propertyName == "visibility") {
        instance->visible = propertyValue.toBool(); // works for visibility either
        // This lets dirty collector to collect dirt and send a render back
        DesignerSupport::addDirty(RenderUtils::guiItem(instance),
                                  DesignerSupport::ContentUpdateMask);
        return;
    }

    if (propertyName == "x"
            || propertyName == "y"
            || propertyName == "width"
            || propertyName == "height") {
        QString hash;
        QVariant actualValue(propertyValue);
        valueFromGeometrySyncKey(&actualValue, &hash);
        property.write(actualValue);
        instance->geometrySyncKey = hash;
    } else {
        property.write(propertyValue);
    }

    DesignerSupport::addDirty(RenderUtils::guiItem(instance), DesignerSupport::ContentUpdateMask);
    if (instance->parent && instance->parent->object)
        DesignerSupport::addDirty(RenderUtils::guiItem(instance->parent), DesignerSupport::ChildrenUpdateMask);
}

void RenderUtils::deleteInstancesRecursive(RenderEngine::ControlInstance* instance,
                                           DesignerSupport& designerSupport)
{
    Q_ASSERT(instance);

    for (RenderEngine::ControlInstance* childInstance : instance->children)
        RenderUtils::deleteInstancesRecursive(childInstance, designerSupport);

    RenderUtils::setId(instance->context, nullptr, instance->id, QString());

    QQuickItem* item = RenderUtils::guiItem(instance->object);

    if (item)
        designerSupport.derefFromEffectItem(item);

    if (instance->object) {
        auto item = RenderUtils::guiItem(instance->object);
        if (item) {
            item->setParentItem(nullptr);
            delete item;
        }
        if (instance->object != item)
            delete instance->object;
    }
    delete instance;
}

void RenderUtils::cleanUpFormInstances(const QList<RenderEngine::ControlInstance*>& formInstances,
                                       QQmlContext* rootContext, DesignerSupport& designerSupport)
{
    for (RenderEngine::ControlInstance* formInstance : formInstances) {
        Q_ASSERT(formInstance);

        for (RenderEngine::ControlInstance* childInstance : formInstance->children)
            RenderUtils::deleteInstancesRecursive(childInstance, designerSupport);

        RenderUtils::setId(rootContext, nullptr, formInstance->id, QString());
        RenderUtils::setId(formInstance->context, nullptr, formInstance->id, QString());

        QQuickItem* item = RenderUtils::guiItem(formInstance->object);

        if (item)
            designerSupport.derefFromEffectItem(item);

        if (formInstance->object) {
            auto item = RenderUtils::guiItem(formInstance->object);
            if (item) {
                item->setParentItem(nullptr);
                delete item;
            }
            if (formInstance->object != item)
                delete formInstance->object;
        }

        delete formInstance->context;
        delete formInstance;
    }
}

void RenderUtils::setId(QQmlContext* context, QObject* object, const QString& oldId,
                        const QString& newId)
{
    if (!oldId.isEmpty() && context)
        context->setContextProperty(oldId, 0);

    if (!newId.isEmpty() && context)
        context->setContextProperty(newId, object);
}

QObject* RenderUtils::parentObject(const RenderEngine::ControlInstance* parentInstance,
                                   const QQuickView* m_view)
{
    QObject* parentObject;
    if (parentInstance->errors.isEmpty()) {
        if (parentInstance->gui)
            parentObject = parentInstance->object;
        else
            parentObject = m_view->rootObject();
    } else {
        parentObject = m_view->rootObject();
    }
    return parentObject;
}

QQuickItem* RenderUtils::guiItem(const RenderEngine::ControlInstance* instance)
{
    Q_ASSERT(instance);

    if (!instance->errors.isEmpty())
        return nullptr;

    if (instance->window)
        return qobject_cast<QQuickWindow*>(instance->object)->contentItem();
    else if (instance->popup)
        return qobject_cast<QQuickPopup*>(instance->object)->popupItem();
    else
        return qobject_cast<QQuickItem*>(instance->object);
}

QQuickItem* RenderUtils::guiItem(QObject* object)
{
    if (!object)
        return nullptr;
    if (object->isWindowType())
        return qobject_cast<QQuickWindow*>(object)->contentItem();
    else if (object->inherits("QQuickPopup"))
        return qobject_cast<QQuickPopup*>(object)->popupItem();
    else
        return qobject_cast<QQuickItem*>(object);
}

QQuickItem* RenderUtils::createDummyItem(QQmlEngine* engine)
{
    QQmlComponent component(engine);
    component.setData("import QtQuick 2.9\nItem{}", QUrl());
    QQuickItem* item = qobject_cast<QQuickItem*>(component.create(engine->rootContext()));
    Q_ASSERT(item);
    item->setFlag(QQuickItem::ItemHasContents, true);
    return item;
}

void RenderUtils::valueFromGeometrySyncKey(QVariant* value, QString* hash)
{
    UtilityFunctions::pull(value->toByteArray(), *value, *hash) ;
}

QVector<PropertyNode> RenderUtils::properties(const RenderEngine::ControlInstance* instance)
{
    Q_ASSERT(instance);
    QVector<PropertyNode> propertyNodes;
    if (!instance->errors.isEmpty())
        return propertyNodes;

    QObject* object = instance->object;
    Q_ASSERT(object);

    QVector<Enum> enums;
    QMap<QString, QVariant> properties;
    auto metaObject = object->metaObject();
    while (metaObject) {
        if (metaObject->propertyOffset() - metaObject->propertyCount() == 0) {
            metaObject = metaObject->superClass();
            continue;
        }

        QString className = cleanClassName(metaObject);
        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
            const auto& property = metaObject->property(i);

            enums << subEnums(property, object);
            properties.unite(subProperties(property, object));

            if (property.isReadable()
                    && property.isWritable()
                    && metaPropertyPasses(property)
                    && !propertyExistsInNodes(propertyNodes, property.name())) {
                if (property.isEnumType() && !property.isFlagType()) {
                    auto metaEnum = property.enumerator();

                    Enum e;
                    e.name = property.name();
                    e.scope = cleanScopeName(metaEnum.scope());
                    e.value = metaEnum.valueToKey(property.read(object).toInt());

                    for (int i = metaEnum.keyCount(); i--;) {
                        if (QString(metaEnum.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
                            e.keys[metaEnum.key(i)] = metaEnum.value(i);
                    }

                    enums << e;
                } else if (!property.isFlagType()) {
                    properties[property.name()] = property.read(object);
                }
            }
        }

        if (!unitePropertiesIfPossible(propertyNodes, className, properties, enums)) {
            PropertyNode propertyNode;
            propertyNode.cleanClassName = className;
            propertyNode.properties = properties;
            propertyNode.enums = enums;
            propertyNodes << propertyNode;
        }

        metaObject = metaObject->superClass();
    }

    return propertyNodes;
}

QMarginsF RenderUtils::margins(const RenderEngine::ControlInstance* instance)
{
    if (instance->gui && instance->codeChanged) {
        const bool completeDisabled = !QQmlVME::componentCompleteEnabled();
        if (completeDisabled)
            QQmlVME::enableComponentComplete();
        QQuickItem* parentItem = RenderUtils::guiItem(instance->object);
        Q_ASSERT(parentItem);
        QQmlComponent com(qmlEngine(instance->object));
        com.setData("import QtQuick 2.7;Item{anchors.fill:parent}", QUrl());
        QQuickItem* item = qobject_cast<QQuickItem*>(com.create(qmlContext(instance->object)));
        Q_ASSERT(item);
        // For calculating QQuickContainer margins right
        QQuickItemPrivate::get(item)->setTransparentForPositioner(true);
        QQmlProperty defaultProperty(instance->object);
        Q_ASSERT(defaultProperty.isValid());
        QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
        Q_ASSERT(childList.isValid() && childList.canAppend());
        childList.append(item);
        const QRectF rect(item->mapRectToItem(parentItem, QRectF(QPointF(), item->size())));
        QMarginsF margins(rect.left(), rect.top(), parentItem->width() - rect.right(),
                          parentItem->height() - rect.bottom());
        item->setParentItem(nullptr);
        delete item;
        if (completeDisabled)
            QQmlVME::disableComponentComplete();
        return margins;
    }
    return QMarginsF();
}

QVariantMap RenderUtils::anchors(const RenderEngine::ControlInstance* instance, const RenderEngine* engine)
{
    QVariantMap anchorMap;
    if (instance->gui) {
        if (instance->popup) { // Exception
            const QQmlProperty prop(instance->object, "anchors.centerIn", instance->context);
            if (prop.isValid()) {
                if (const QObject* targetObject = prop.read().value<QObject*>()) {
                    if (const RenderEngine::ControlInstance* targetInstance = engine->instanceForObject(targetObject)) {
                        anchorMap.insert("anchors.centerIn", QStringList(targetInstance->uid));
                    } else if (auto targetItem = qobject_cast<const QQuickItem*>(targetObject)) {
                        if (const RenderEngine::ControlInstance* ancestorInstance = engine->findNodeInstanceForItem(targetItem->parentItem()))
                            anchorMap.insert("anchors.centerIn", QStringList(ancestorInstance->uid));
                    }
                }
            }
        } else if (!instance->window) {
            // Windows cannot have anchors, but they can be targets for other anchors
            for (const QString& name : UtilityFunctions::anchorLineNames()) {
                const QStringList& anchorPair = makeAnchorPair(instance, name, engine);
                if (!anchorPair.isEmpty())
                    anchorMap.insert(name, anchorPair);
            }
            for (const QString& name : UtilityFunctions::anchorPropertyNames()) {
                const QQmlProperty prop(instance->object, name, instance->context);
                if (prop.isValid())
                    anchorMap.insert(name, prop.read());
            }
            if (qreal margins = anchorMap.value("anchors.margins").toReal()) {
                if (anchorMap.value("anchors.leftMargin").toReal() == margins
                        && !ParserUtils::exists(instance->dir, "anchors.leftMargin")) {
                    anchorMap.insert("anchors.leftMargin", 0);
                } if (anchorMap.value("anchors.rightMargin").toReal() == margins
                      && !ParserUtils::exists(instance->dir, "anchors.rightMargin")) {
                    anchorMap.insert("anchors.rightMargin", 0);
                } if (anchorMap.value("anchors.topMargin").toReal() == margins
                      && !ParserUtils::exists(instance->dir, "anchors.topMargin")) {
                    anchorMap.insert("anchors.topMargin", 0);
                } if (anchorMap.value("anchors.bottomMargin").toReal() == margins
                      && !ParserUtils::exists(instance->dir, "anchors.bottomMargin")) {
                    anchorMap.insert("anchors.bottomMargin", 0);
                }
            }
        }
    }
    return anchorMap;
}

QStringList RenderUtils::makeAnchorPair(const RenderEngine::ControlInstance* instance, const QString& name, const RenderEngine* engine)
{
    QQmlContext* ctx = instance->context;
    Q_ASSERT(ctx);
    QQuickItem* item = qobject_cast<QQuickItem*>(instance->object);
    if (item == 0)
        return QStringList();
    const QPair<QString, QObject*>& pair = DesignerSupport::anchorLineTarget(item, name, ctx);
    if (const RenderEngine::ControlInstance* targetInstance = engine->instanceForObject(pair.second)) {
        if (pair.first.isEmpty())
            return QStringList(targetInstance->uid);
        else
            return QStringList({pair.first, targetInstance->uid});
    } else if (auto targetItem = qobject_cast<const QQuickItem*>(pair.second)) {
        if (const RenderEngine::ControlInstance* ancestorInstance = engine->findNodeInstanceForItem(targetItem->parentItem())) {
            if (pair.first.isEmpty())
                return QStringList(ancestorInstance->uid);
            else
                return QStringList({pair.first, ancestorInstance->uid});
        }
    }
    return QStringList();
}
