#include <renderutils.h>
#include <saveutils.h>
#include <renderresult.h>
#include <utilityfunctions.h>
#include <parserutils.h>

#include <QAnimationDriver>
#include <QQuickWindow>
#include <QQmlContext>

#include <private/qqmltimer_p.h>
#include <private/qquicktext_p.h>
#include <private/qqmlengine_p.h>
#include <private/qquicktextedit_p.h>
#include <private/qquickanimation_p.h>
#include <private/qquicktextinput_p.h>
#include <private/qquicktransition_p.h>
#include <private/qquickdesignersupport_p.h>
#include <private/qquickpopup_p.h>

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
    foreach (QQuickItem* childItem, item->childItems())
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
    foreach (QObject *childObject, object->children())
        allSubObject(childObject, objectList);

    // search recursive in quick item childItems list
    QQuickItem *quickItem = qobject_cast<QQuickItem*>(object);
    if (quickItem) {
        foreach (QQuickItem *childItem, quickItem->childItems()) {
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

        QList<QObject*> childList = object->children();

        if (item) {
            foreach (QQuickItem *childItem, item->childItems()) {
                if (!childList.contains(childItem))
                    childList.append(childItem);
            }
        }

        foreach (QObject *child, childList) {
            if (!engine->hasInstanceForObject(child))
                doComponentCompleteRecursive(child, engine);
        }

        if (item) {
            static_cast<QQmlParserStatus*>(item)->componentComplete();
        } else {
            QQmlParserStatus *qmlParserStatus = dynamic_cast< QQmlParserStatus*>(object);
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

QVariant RenderUtils::evaluate(const RenderEngine::ControlInstance* instance, const QString& binding)
{
    QQmlExpression expr(instance->context, instance->object, binding);
    return expr.evaluate();
}

bool RenderUtils::isVisible(const RenderEngine::ControlInstance* instance)
{
    if (instance->gui) {
        bool visible;
        if (instance->object->isWindowType())
            visible = evaluate(instance, ParserUtils::property(instance->dir, "visible")).toBool();
        else
            visible = QQmlProperty::read(instance->object, "visible").toBool();
        if (instance->object->isWindowType())
            return visible && !ParserUtils::property(instance->dir, "visibility").contains("Hidden");
        return visible;
    }
    return false;
}

bool RenderUtils::isRectangleSane(const QRectF& rect)
{
    return rect.isValid() && (rect.width() < 10000) && (rect.height() < 10000);
}

void RenderUtils::refreshLayoutable(RenderEngine::ControlInstance* instance)
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
        DesignerSupport::emitComponentCompleteSignalForAttachedProperty(item);
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

    foreach (QQuickItem *childItem, parentItem->childItems()) {
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
        DesignerSupport::addDirty(RenderUtils::guiItem(instance), DesignerSupport::ContentUpdateMask);
        return;
    }

    property.write(propertyValue);

    DesignerSupport::addDirty(RenderUtils::guiItem(instance), DesignerSupport::ContentUpdateMask);
    if (instance->parent && instance->parent->object) {
        DesignerSupport::addDirty(RenderUtils::guiItem(instance->parent), DesignerSupport::ChildrenUpdateMask);
        refreshLayoutable(instance->parent);
    }
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

    if (instance->object)
        delete instance->object;

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

        if (formInstance->object)
            delete formInstance->object;

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

QQuickItem* RenderUtils::guiItem(RenderEngine::ControlInstance* instance)
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

    if (instance->gui && instance->codeChanged) {
        QQuickItem* parentItem = RenderUtils::guiItem(object);
        Q_ASSERT(parentItem);
        QQmlComponent com(qmlEngine(object));
        com.setData("import QtQuick 2.7;Item{anchors.fill:parent}", QUrl());
        QQuickItem* item = qobject_cast<QQuickItem*>(com.create(qmlContext(object)));
        Q_ASSERT(item);
        QQmlProperty defaultProperty(object);
        Q_ASSERT(defaultProperty.isValid());
        QQmlListReference childList = defaultProperty.read().value<QQmlListReference>();
        childList.append(item);
        const QRectF rect(item->mapToItem(parentItem, {0, 0}), item->size());
        QMarginsF margins(rect.left(), rect.top(), parentItem->width() - rect.right(),
                          parentItem->height() - rect.bottom());
        UtilityFunctions::putMarginsToProperties(properties, margins);
        item->deleteLater();
    }

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
