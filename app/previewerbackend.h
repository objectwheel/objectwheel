#ifndef PREVIEWERBACKEND_H
#define PREVIEWERBACKEND_H

#include <QObject>
#include <QRectF>
#include <QVariant>

class QLocalServer;
struct PreviewResult;
class QDataStream;
class QTimer;

class PreviewerBackend : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(PreviewerBackend)

    private:
        struct Task
        {
            enum Type
            {
                Init,
                Preview,
                Repreview,
                Update,
                Remove,
                Reparent
            };

            inline bool operator==(const Task& t1)
            {
                return uid == t1.uid && type == t1.type && property == t1.property;
            }

            Type type;
            QSizeF size;
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
        bool init();
        bool isBusy() const;
        bool contains(const QString& uid) const;
        int totalTask() const;
        void setDisabled(bool value);

    public slots:
        void restart();
        void requestInit(const QString& projectDir);
        void requestPreview(const QSizeF& size, const QString& dir, bool repreview = false);
        void removeCache(const QString& uid);
        void updateParent(const QString& uid, const QString& parentUid, const QString& newUrl);
        void updateCache(const QString& uid, const QString& property, const QVariant& value);

    private slots:
        void disableDirtHandling();
        void enableDirtHandling();
        void onNewConnection();
        void onReadReady();
        void onBinaryMessageReceived(const QByteArray& data);

    private:
        void processNextTask();
        void processMessage(const QString& type, QDataStream& in);

    signals:
        void taskDone();
        void busyChanged();
        void previewReady(const PreviewResult& result);

    private:
        PreviewerBackend();
        ~PreviewerBackend();

    private:
        bool m_disabled;
        QLocalServer* m_server;
        QTimer* m_dirtHandlingDisablerTimer;
        QList<Task> m_taskList;
        bool m_dirtHandlingEnabled;
};

#endif // PREVIEWERBACKEND_H
