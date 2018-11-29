#ifndef PREVIEWER_H
#define PREVIEWER_H

#include <QSet>
#include <QObject>

#include <private/qquickdesignersupport_p.h>

class QQuickView;
class QQmlError;
class QQuickWindow;
class QQmlComponent;
struct PreviewResult;

// Due to possible margins on an ApplicationWindow the previewing order is important
// And a parent must be always previewed first.
template <typename T>
class OnlyOneInstanceList : public QList<T>
{
public:
    inline void insert(const T& t) { if (!this->contains(t)) this->append(t); }
    inline void remove(const T& t) { this->removeOne(t); }
};

class Previewer final : public QObject
{
    Q_OBJECT

public:
    struct ControlInstance {
        bool gui;
        bool popup;
        bool window;
        bool needsRepreview = false;
        bool codeChanged = false;
        QString id;
        QString uid;
        QString dir;
        QObject* object;
        QQmlContext* context;
        QQmlComponent* component;
        QList<QQmlError> errors;

        ControlInstance* parent = nullptr;
        QList<ControlInstance*> children;
    };

public:
    explicit Previewer(QObject* parent = nullptr);
    ~Previewer();

public:
    bool hasInstanceForObject(const QObject* object) const;
    bool hasInstanceForUid(const QString& uid) const;

    ControlInstance* instanceForObject(const QObject* object) const;
    ControlInstance* instanceForUid(const QString& uid) const;
    ControlInstance* formInstanceFor(const ControlInstance* instance);
    ControlInstance* findNodeInstanceForItem(QQuickItem* item) const;

public slots:
    void init();
    void refresh(const QString& formUid);
    void deleteForm(const QString& uid); // NOTE: Deletes form and its children
    void deleteControl(const QString& uid); // NOTE: Deletes control and its children
    void createForm(const QString& dir); // NOTE: It doesn't create children
    void createControl(const QString& dir, const QString& parentUid); // NOTE: It doesn't create children
    void updateId(const QString& uid, const QString& newId);
    void updateParent(const QString& newDir, const QString& uid, const QString& parentUid);
    void updateProperty(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    void updateControlCode(const QString& uid);
    void updateFormCode(const QString& uid);

private:
    void refreshBindings();
    void preview(ControlInstance* formInstance);
    void schedulePreview(ControlInstance* formInstance, int msecLater = 100);
    void scheduleRepreviewForInvisibleInstances(ControlInstance* formInstance, int msecLater = 500);

    QImage grabImage(const ControlInstance* instance);
    QImage renderItem(QQuickItem* item, const QColor& bgColor);

    QList<PreviewResult> previewDirtyInstances(const QList<ControlInstance*>& instances);
    Previewer::ControlInstance* createInstance(const QString& dir, ControlInstance* parentInstance,
                                               QQmlContext* oldFormContext = nullptr);

signals:
    void initializationProgressChanged(int progress);
    void previewDone(const QList<PreviewResult>& results);

private:
    bool m_initialized;
    DesignerSupport m_designerSupport;
    QList<ControlInstance*> m_formInstances;
    OnlyOneInstanceList<ControlInstance*> m_dirtyInstanceSet;
    QQuickView* m_view;
};

#endif // PREVIEWER_H