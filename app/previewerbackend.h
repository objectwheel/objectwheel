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
                Remove
            };

            inline bool operator==(const Task& t1)
            {
                return uid == t1.uid && type == t1.type && property == t1.property;
            }

            Type type;
            QSizeF size;
            QString uid, dir;
            QString property;
            QVariant propertyValue;
            bool needsUpdate = false;
        };

    public:
        static PreviewerBackend* instance();
        bool init();
        bool isBusy() const;
        int totalTask() const;

    public slots:
        void restart();
        void requestInit(const QString& projectDir);
        void requestPreview(const QSizeF& size, const QString& dir, bool repreview = false);
        void removeCache(const QString& uid);
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
        QLocalServer* _server;
        QTimer* _dirtHandlingDisablerTimer;
        QList<Task> _taskList;
        bool _dirtHandlingEnabled;
};

#endif // PREVIEWERBACKEND_H