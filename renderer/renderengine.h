#ifndef RENDERENGINE_H
#define RENDERENGINE_H

#include <QVariant>
#include <QVector>
#include <private/qquickdesignersupport_p.h>

class QQuickView;
class QmlError;
class QQuickWindow;
class QTimer;
struct RenderInfo;

// Due to possible margins on an ApplicationWindow the rendering order is important
// And a parent must always be rendered first.
template <typename T>
class OnlyOneInstanceList final : public QList<T>
{
public:
    void insert(const T& t) { if (!QList<T>::contains(t)) this->append(t); }
    void remove(const T& t) { QList<T>::removeOne(t); }
};

class RenderEngine final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RenderEngine)

public:
    struct ControlInstance {
        bool gui;
        bool popup;
        bool window;
        bool visible = true;
        bool layout = false;
        bool needsRerender = false;
        bool codeChanged = false;
        bool preview = false;
        QString id;
        QString uid;
        QString dir;
        QString geometrySyncKey;
        QObject* object;
        QQmlContext* context;
        QVector<QmlError> errors;

        ControlInstance* parent = nullptr;
        OnlyOneInstanceList<ControlInstance*> children;
    };

public:
    explicit RenderEngine(QObject* parent = nullptr);
    ~RenderEngine() override;

public:
    bool hasInstanceForObject(const QObject* object) const;
    bool hasInstanceForUid(const QString& uid) const;

    ControlInstance* instanceForObject(const QObject* object) const;
    ControlInstance* instanceForUid(const QString& uid) const;
    ControlInstance* formInstanceFor(const ControlInstance* instance);
    ControlInstance* findNodeInstanceForItem(const QQuickItem* item) const;

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(qreal devicePixelRatio);

public slots:
    void init();
    void refresh(const QString& formUid);
    void deleteForm(const QString& uid); // NOTE: Deletes form and its children
    void deleteControl(const QString& uid); // NOTE: Deletes control and its children
    void createForm(const QString& dir); // NOTE: It doesn't create children
    void createControl(const QString& dir, const QString& parentUid); // NOTE: It doesn't create children
    void updateIndex(const QString& uid);
    void updateId(const QString& uid, const QString& newId);
    void updateParent(const QString& newDir, const QString& uid, const QString& parentUid);
    void updateProperty(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    void updateControlCode(const QString& uid);
    void updateFormCode(const QString& uid);
    void preview(const QString& url);

private slots:
    void flushRenders();
    void flushReRenders();

private:
    void refreshAllBindings();
    void refreshBindings(QQmlContext* context);
    void repairIndexes(ControlInstance* parentInstance);
    void scheduleRender(ControlInstance* formInstance);
    void scheduleRerenderForInvisibleInstances(ControlInstance* formInstance);

    QRectF boundingRectWithStepChilds(QQuickItem* item);
    QRectF boundingRect(QQuickItem* item);

    QImage grabImage(const ControlInstance* instance, QRectF& boundingRect);
    QImage renderItem(QQuickItem* item, QRectF& boundingRect, bool preview, const QColor& bgColor);

    QList<RenderInfo> renderDirtyInstances(const QList<ControlInstance*>& instances);
    RenderEngine::ControlInstance* createInstance(const QString& url);
    RenderEngine::ControlInstance* createInstance(const QString& dir, ControlInstance* parentInstance,
                                                  QQmlContext* oldFormContext = nullptr);

signals:
    void initializationProgressChanged(int progress);
    void renderDone(const QList<RenderInfo>& infos);
    void previewDone(const RenderInfo& info);

private:
    bool m_initialized;
    qreal m_devicePixelRatio;
    DesignerSupport m_designerSupport;
    QList<ControlInstance*> m_formInstances;
    OnlyOneInstanceList<ControlInstance*> m_dirtyInstanceSet;
    OnlyOneInstanceList<ControlInstance*> m_formInstanceSetForRender;
    OnlyOneInstanceList<ControlInstance*> m_formInstanceSetForReRender;
    QQuickView* m_view;
    QTimer* m_renderTimer;
    QTimer* m_reRenderTimer;
};

#endif // RENDERENGINE_H