#ifndef PREVIEWERBACKEND_H
#define PREVIEWERBACKEND_H

#include <QObject>
#include <QRectF>
#include <QVariant>

class QLocalServer;
struct PreviewResult;
class QDataStream;
class QTimer;

class PreviewerBackend final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PreviewerBackend)

    friend class BackendManager;

private:
    struct Task
    {
        enum Type {
            Init,
            Preview,
            Repreview,
            Update,
            Remove,
            Reparent
        };

        inline bool operator==(const Task& t1)
        { return uid == t1.uid && type == t1.type && property == t1.property; }

        Type type;
        QString dir;
        QString uid;
        QString newUrl;
        QString parentUid;
        QString property;
        QVariant propertyValue;
        bool needsUpdate = false;
    };

public:
    static PreviewerBackend* instance();

    static bool isBusy();
    static bool contains(const QString& uid);
    static int totalTask();

    static bool init();
    static void restart();
    static void setDisabled(bool value);
    static void requestInit(const QString& projectDir);
    static void removeCache(const QString& uid);
    static void requestPreview(const QString& dir, bool repreview = false);
    static void updateParent(const QString& uid, const QString& parentUid, const QString& newUrl);
    static void updateCache(const QString& uid, const QString& property, const QVariant& value);

private:
    static void onReadReady();
    static void processNextTask();
    static void onNewConnection();
    static void enableDirtHandling();
    static void disableDirtHandling();
    static void onBinaryMessageReceived(const QByteArray& data);
    static void fixTasksAgainstReparent(const QString& uid, const QString& newUrl);
    static void processMessage(const QString& type, QDataStream& in);

signals:
    void taskDone();
    void busyChanged();
    void previewReady(const PreviewResult& result);

private:
    explicit PreviewerBackend(QObject* parent = nullptr);
    ~PreviewerBackend();

private:
    static PreviewerBackend* s_instance;
    static bool s_disabled;
    static bool s_dirtHandlingEnabled;
    static QLocalServer* s_server;
    static QTimer* s_dirtHandlingDisablerTimer;
    static QList<Task> s_taskList;
};

#endif // PREVIEWERBACKEND_H
