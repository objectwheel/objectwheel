#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#include <renderengine.h>

#include <QtGlobal>

class QQmlEngine;
class QQuickItem;
struct PropertyNode;

class RenderUtils final
{
    Q_DISABLE_COPY(RenderUtils)

public:
    static QObject* parentObject(const RenderEngine::ControlInstance* parentInstance, const QQuickView* m_view);
    static QQuickItem* guiItem(const RenderEngine::ControlInstance* instance);
    static QQuickItem* guiItem(QObject* object);
    static QQuickItem* createDummyItem(QQmlEngine* engine);
    static void valueFromGeometrySyncKey(QVariant* value, QString* hash);
    static QVariant evaluate(const RenderEngine::ControlInstance* instance, const QString& expression);
    static QList<QQuickItem*> allItems(RenderEngine::ControlInstance* formInstance);
    static QList<RenderEngine::ControlInstance*> allSubInstance(RenderEngine::ControlInstance* parentInstance);
    static QVector<QString> events(const RenderEngine::ControlInstance* instance);
    static QVector<PropertyNode> properties(const RenderEngine::ControlInstance* instance);
    static QMarginsF margins(const RenderEngine::ControlInstance* instance);
    static QVariantMap anchors(const RenderEngine::ControlInstance* instance, const RenderEngine* engine);
    static QStringList makeAnchorPair(const RenderEngine::ControlInstance* instance,
                                      const QString& name, const RenderEngine* engine);

    static int countAllSubInstance(const RenderEngine::ControlInstance* parentInstance);
    static bool isOverlayPopup(const RenderEngine::ControlInstance* instance);
    static bool isVisible(const RenderEngine::ControlInstance* instance);
    static bool isRectangleSane(const QRectF& rect);
    static void setInstanceParent(RenderEngine::ControlInstance* instance, QObject* parentObject);
    static void refreshLayoutable(const RenderEngine::ControlInstance* instance);
    static void updateDirtyNodesRecursive(QQuickItem* parentItem, RenderEngine* engine);
    static void doComplete(RenderEngine::ControlInstance* instance, const RenderEngine* engine);
    static void tweakObjects(QObject* object);
    static void resetAllItems(RenderEngine::ControlInstance* formInstance);
    static void disableNativeTextRendering(QQuickItem* item);
    static void dash(const QSizeF& size, QImage& image);
    static void stopUnifiedTimer();
    static void makeDirtyRecursive(RenderEngine::ControlInstance* beginningInstance);
    static void setInstancePropertyBinding(const RenderEngine::ControlInstance* instance,
                                           const QString& bindingName,
                                           const QString& expression);
    static void setInstancePropertyVariant(RenderEngine::ControlInstance* instance,
                                           const QString& propertyName,
                                           const QVariant& propertyValue);
    static void deleteInstancesRecursive(RenderEngine::ControlInstance* instance,
                                         DesignerSupport& designerSupport);
    static void cleanUpFormInstances(const QList<RenderEngine::ControlInstance*>& formInstances,
                                     QQmlContext* rootContext, DesignerSupport& designerSupport);
    static void setId(QQmlContext* rootContext, QObject* object,
                      const QString& oldId, const QString& newId);
private:
    RenderUtils() {}
};

#endif // RENDERUTILS_H