#ifndef PREVIEWERBACKEND_H
#define PREVIEWERBACKEND_H

#include <QObject>
#include <QRectF>
#include <QProcess>

class QLocalSocket;
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
            QString dir;
            bool repreview;

            bool operator==(const Task& t1)
            {
                return dir == t1.dir;
            }
        };

    public:
        static PreviewerBackend* instance();
        bool isReady() const;
        bool isWorking() const;

    public slots:
        void start();
        void restart();
        void requestPreview(const QRectF& rect, const QString& dir, bool repreview = false);

    private slots:
        void onProcessStart();
        void onProcessFinish(int, QProcess::ExitStatus);

        void onSocketReadReady();
        void onSocketBinaryMessageReceived(const QByteArray& data);

    private:
        void waitForReady();
        void connectToServer();
        void processNextTask();
        void processMessage(const QString& type, QDataStream& in);

    signals:
        void stateChanged();
        void previewReady(const PreviewResult& result);

    private:
        PreviewerBackend();

    private:
        QProcess* _process;
        QLocalSocket* _socket;
        QList<Task> _taskList;
};

#endif // PREVIEWERBACKEND_H