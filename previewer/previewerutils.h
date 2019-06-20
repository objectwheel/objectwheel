#ifndef PREVIEWERUTILS_H
#define PREVIEWERUTILS_H

#include <previewer.h>

#include <QtGlobal>

class QQmlEngine;
class QQuickItem;
struct PropertyNode;

class PreviewerUtils final
{
    Q_DISABLE_COPY(PreviewerUtils)

public:
    static QQuickItem* guiItem(Previewer::ControlInstance* instance);
    static QQuickItem* guiItem(QObject* object);
    static QQuickItem* createDummyItem(QQmlEngine* engine);

    static QList<QQuickItem*> allItems(Previewer::ControlInstance* formInstance);
    static QList<QString> events(const Previewer::ControlInstance* instance);
    static QList<PropertyNode> properties(const Previewer::ControlInstance* instance);
    static QList<Previewer::ControlInstance*> allSubInstance(Previewer::ControlInstance* parentInstance);
    static int countAllSubInstance(const Previewer::ControlInstance* parentInstance);

    static void updateDirtyNodesRecursive(QQuickItem* parentItem, Previewer* previewer);
    static void doComplete(Previewer::ControlInstance* instance, const Previewer* previewer);
    static void tweakObjects(QObject* object);
    static void resetAllItems(Previewer::ControlInstance* formInstance);
    static void disableNativeTextRendering(QQuickItem* item);
    static void dash(const QSizeF& size, QImage& image);
    static void stopUnifiedTimer();
    static void makeDirtyRecursive(Previewer::ControlInstance* beginningInstance);
    static void setInstancePropertyVariant(Previewer::ControlInstance* instance,
                                           const QString& propertyName,
                                           const QVariant& propertyValue);
    static void deleteInstancesRecursive(Previewer::ControlInstance* instance,
                                         DesignerSupport& designerSupport);
    static void cleanUpFormInstances(const QList<Previewer::ControlInstance*>& formInstances,
                                     QQmlContext* rootContext, DesignerSupport& designerSupport);
    static void setId(QQmlContext* rootContext, QObject* object,
                      const QString& oldId, const QString& newId);
private:
    PreviewerUtils() {}
};

#endif // PREVIEWERUTILS_H