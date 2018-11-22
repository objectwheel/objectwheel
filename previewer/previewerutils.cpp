#include <previewerutils.h>
#include <previewer.h>
#include <saveutils.h>
#include <filemanager.h>
#include <previewresult.h>

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

void doComponentCompleteRecursive(QObject* object, const Previewer* previewer)
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
            if (!previewer->hasInstanceForObject(child))
                doComponentCompleteRecursive(child, previewer);
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

bool propertyExistsInNodes(const QList<PropertyNode>& nodes, const QString& property)
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

bool unitePropertiesIfPossible(QList<PropertyNode>& nodes, const QString& className,
                               const QMap<QString, QVariant>& properties, const QList<Enum>& enums)
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

QList<Enum> subEnums(QMetaProperty property, const QObject* parentObject)
{
    QList<Enum> enums;

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

QList<QQuickItem*> PreviewerUtils::allItems(Previewer::ControlInstance* formInstance)
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

    for (Previewer::ControlInstance* instance : PreviewerUtils::allSubInstance(formInstance)) {
        Q_ASSERT(instance);
        if (instance->gui) {
            QQuickItem* item = PreviewerUtils::guiItem(instance);
            Q_ASSERT(item);
            if (!itemList.contains(item)) {
                itemList.append(item);
                itemList.append(allChildItemsRecursive(item));
            }
        }
    }

    return itemList;
}

QList<QString> PreviewerUtils::events(const Previewer::ControlInstance* instance)
{
    Q_ASSERT(instance);
    QList<QString> events;
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

QList<Previewer::ControlInstance*> PreviewerUtils::allSubInstance(Previewer::ControlInstance* parentInstance)
{
    QList<Previewer::ControlInstance*> instances;
    if (!parentInstance)
        return instances;

    instances.append(parentInstance);

    for (Previewer::ControlInstance* childInstance : parentInstance->children)
        instances.append(allSubInstance(childInstance));

    return instances;
}

int PreviewerUtils::countAllSubInstance(const Previewer::ControlInstance* parentInstance)
{
    int counter = 0;

    if (!parentInstance)
        return counter;

    ++counter;

    for (Previewer::ControlInstance* childInstance : parentInstance->children)
        counter += countAllSubInstance(childInstance);

    return counter;
}

bool PreviewerUtils::needsRepreview(const QImage& image)
{
    if (image.isNull())
        return true;

    int totalAlpha = 200 * 8;
    const int hspacing = 8;
    const int wspacing = qRound(qMax(hspacing * qreal(image.width()) / image.height(), 1.0));

    for (int w = 0, h = image.height() / 2.0; w < image.width(); w += wspacing)
        totalAlpha -= qAlpha(image.pixel(w, h));

    if (totalAlpha < 0)
        return false;

    for (int w = 0, h = 0; w < image.width() && h < image.height(); w += wspacing, h += hspacing)
        totalAlpha -= qAlpha(image.pixel(w, h));

    if (totalAlpha < 0)
        return false;

    for (int w = image.width() - 1, h = 0; w >= 0 && h < image.height(); w -= wspacing, h += hspacing)
        totalAlpha -= qAlpha(image.pixel(w, h));

    return totalAlpha > 0;
}

void PreviewerUtils::resetAllItems(Previewer::ControlInstance* formInstance)
{
    Q_ASSERT(formInstance);
    for (QQuickItem* item : allItems(formInstance))
        DesignerSupport::resetDirty(item);
}

void PreviewerUtils::doComplete(Previewer::ControlInstance* instance, const Previewer* previewer)
{
    Q_ASSERT(instance);
    if (!instance->errors.isEmpty())
        return;

    Q_ASSERT(instance->object);
    doComponentCompleteRecursive(instance->object, previewer);

    // FIXME: How do we handle header, footer or toolbar items of an ApplicationWindow?
    if (instance->gui) {
        auto item = qobject_cast<QQuickItem*>(instance->object);

        if (!item)
            item = qobject_cast<QQuickWindow*>(instance->object)->contentItem();

        disableTextCursor(item);
        DesignerSupport::emitComponentCompleteSignalForAttachedProperty(item);
        item->update();
    }
}

void PreviewerUtils::tweakObjects(QObject* object)
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

void PreviewerUtils::updateDirtyNodesRecursive(QQuickItem* parentItem, Previewer* previewer)
{
    Q_ASSERT(previewer);
    Q_ASSERT(parentItem);

    foreach (QQuickItem *childItem, parentItem->childItems()) {
        if (!previewer->hasInstanceForObject(childItem))
            updateDirtyNodesRecursive(childItem, previewer);
    }

    PreviewerUtils::disableNativeTextRendering(parentItem);
    DesignerSupport::updateDirtyNode(parentItem);
}

void PreviewerUtils::disableNativeTextRendering(QQuickItem* item)
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

void PreviewerUtils::stopUnifiedTimer()
{
    QUnifiedTimer::instance()->setSlowdownFactor(0.00001);
    QUnifiedTimer::instance()->setSlowModeEnabled(true);
}

void PreviewerUtils::makeDirtyRecursive(Previewer::ControlInstance* beginningInstance)
{
    QQuickItem* item = guiItem(beginningInstance->object);

    if (item)
        DesignerSupport::addDirty(item, DesignerSupport::AllMask);

    for (Previewer::ControlInstance* childInstance : beginningInstance->children)
        makeDirtyRecursive(childInstance);
}

void PreviewerUtils::setInstancePropertyVariant(Previewer::ControlInstance* instance,
                                                const QString& propertyName,
                                                const QVariant& propertyValue)
{
    Q_ASSERT(instance);
    Q_ASSERT(instance->errors.isEmpty());
    Q_ASSERT(instance->object);

    QVariant adjustedValue(propertyValue);
    QQmlProperty property(instance->object, propertyName, instance->context);

    if (!property.isValid())
        return;

    DesignerSupport::addDirty(PreviewerUtils::guiItem(instance),
                              DesignerSupport::ContentUpdateMask);

    if (instance->window && (propertyName == "visible" || propertyName == "visibility"))
        return;

    bool isWritten = property.write(adjustedValue);

    if (!isWritten) {
        qDebug() << "setInstancePropertyVariant: Cannot be written: "
                 << instance->object << propertyName << adjustedValue;
    }
}

void PreviewerUtils::deleteInstancesRecursive(Previewer::ControlInstance* instance,
                                              DesignerSupport& designerSupport)
{
    Q_ASSERT(instance);
    Q_ASSERT(instance->parent);

    for (Previewer::ControlInstance* childInstance : instance->children)
        PreviewerUtils::deleteInstancesRecursive(childInstance, designerSupport);

    PreviewerUtils::setId(instance->context, nullptr, instance->id, QString());

    QQuickItem* item = PreviewerUtils::guiItem(instance->object);

    if (item)
        designerSupport.derefFromEffectItem(item);

    if (instance->object)
        delete instance->object;

    delete instance;
}

void PreviewerUtils::cleanUpFormInstances(const QList<Previewer::ControlInstance*>& formInstances,
                                          QQmlContext* rootContext, DesignerSupport& designerSupport)
{
    for (Previewer::ControlInstance* formInstance : formInstances) {
        Q_ASSERT(formInstance);

        for (Previewer::ControlInstance* childInstance : formInstance->children)
            PreviewerUtils::deleteInstancesRecursive(childInstance, designerSupport);

        PreviewerUtils::setId(rootContext, nullptr, formInstance->id, QString());
        PreviewerUtils::setId(formInstance->context, nullptr, formInstance->id, QString());

        QQuickItem* item = PreviewerUtils::guiItem(formInstance->object);

        if (item)
            designerSupport.derefFromEffectItem(item);

        if (formInstance->object)
            delete formInstance->object;

        delete formInstance->context;
        delete formInstance;
    }
}

void PreviewerUtils::setId(QQmlContext* context, QObject* object, const QString& oldId,
                           const QString& newId)
{
    if (!oldId.isEmpty() && context)
        context->setContextProperty(oldId, 0);

    if (!newId.isEmpty() && context)
        context->setContextProperty(newId, object);
}

QQuickItem* PreviewerUtils::guiItem(Previewer::ControlInstance* instance)
{
    Q_ASSERT(instance);

    if (!instance->errors.isEmpty())
        return nullptr;

    if (instance->window)
        return qobject_cast<QQuickWindow*>(instance->object)->contentItem();
    else
        return qobject_cast<QQuickItem*>(instance->object);
}

QQuickItem* PreviewerUtils::guiItem(QObject* object)
{
    if (!object)
        return nullptr;

    if (object->isWindowType())
        return qobject_cast<QQuickWindow*>(object)->contentItem();
    else
        return qobject_cast<QQuickItem*>(object);
}

QQuickItem* PreviewerUtils::createDummyItem(QQmlEngine* engine)
{
    QQmlComponent component(engine);
    component.setData("import QtQuick 2.9\nItem{}", QUrl());

    QQuickItem* item = qobject_cast<QQuickItem*>(component.create(engine->rootContext()));

    Q_ASSERT(item);

    item->setFlag(QQuickItem::ItemHasContents, true);
    return item;
}

QList<PropertyNode> PreviewerUtils::properties(const Previewer::ControlInstance* instance)
{
    Q_ASSERT(instance);
    QList<PropertyNode> propertyNodes;
    if (!instance->errors.isEmpty())
        return propertyNodes;

    QObject* object = instance->object;
    Q_ASSERT(object);

    QList<Enum> enums;
    QMap<QString, QVariant> properties;

    if (instance->gui && instance->codeChanged) {
        QQuickItem* parentItem = PreviewerUtils::guiItem(object);
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
        properties["__ow_margins_left"] = rect.left();
        properties["__ow_margins_top"] = rect.top();
        properties["__ow_margins_right"] = parentItem->width() - rect.right();
        properties["__ow_margins_bottom"] = parentItem->height() - rect.bottom();
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
