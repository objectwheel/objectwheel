#ifndef PREVIEWERBACKEND_H
#define PREVIEWERBACKEND_H

#include <QObject>
#include <QRectF>

class QLocalServer;
class PreviewResult;
class QDataStream;

class PreviewerBackend : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(PreviewerBackend)

    private:
        struct Task
        {
            QRectF rect;
            QString uid, dir;
            bool repreview;
            bool needsUpdate = false;

            bool operator==(const Task& t1)
            {
                return uid == t1.uid;
            }
        };

    public:
        static PreviewerBackend* instance();
        bool init();
        bool isBusy() const;

    public slots:
        void restart();
        void requestPreview(const QRectF& rect, const QString& dir, bool repreview = false);

    private slots:
        void onNewConnection();
        void onReadReady();
        void onBinaryMessageReceived(const QByteArray& data);

    private:
        void processNextTask();
        void processMessage(const QString& type, QDataStream& in);

    signals:
        void busyChanged();
        void previewReady(const PreviewResult& result);

    private:
        PreviewerBackend();
        ~PreviewerBackend();

    private:
        QLocalServer* _server;
        QList<Task> _taskList;
};

#endif // PREVIEWERBACKEND_H