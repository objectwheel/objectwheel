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
    static QQuickItem* guiItem(RenderEngine::ControlInstance* instance);
    static QQuickItem* guiItem(QObject* object);
    static QQuickItem* createDummyItem(QQmlEngine* engine);

    static QVariant evaluate(const RenderEngine::ControlInstance* instance, const QString& binding);
    static QList<QQuickItem*> allItems(RenderEngine::ControlInstance* formInstance);
    static QVector<QString> events(const RenderEngine::ControlInstance* instance);
    static QVector<QString> blockedPropertyChanges(RenderEngine::ControlInstance* instance);
    static QVector<PropertyNode> properties(const RenderEngine::ControlInstance* instance);
    static QList<RenderEngine::ControlInstance*> allSubInstance(RenderEngine::ControlInstance* parentInstance);
    static int countAllSubInstance(const RenderEngine::ControlInstance* parentInstance);
    static bool isVisible(const RenderEngine::ControlInstance* instance);

    static void refreshLayoutable(RenderEngine::ControlInstance* instance);
    static void updateDirtyNodesRecursive(QQuickItem* parentItem, RenderEngine* engine);
    static void doComplete(RenderEngine::ControlInstance* instance, const RenderEngine* engine);
    static void tweakObjects(QObject* object);
    static void resetAllItems(RenderEngine::ControlInstance* formInstance);
    static void disableNativeTextRendering(QQuickItem* item);
    static void dash(const QSizeF& size, QImage& image);
    static void stopUnifiedTimer();
    static void makeDirtyRecursive(RenderEngine::ControlInstance* beginningInstance);
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