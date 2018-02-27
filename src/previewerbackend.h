#ifndef PREVIEWERBACKEND_H
#define PREVIEWERBACKEND_H

#include <QObject>
#include <QDataStream>
#include <QRectF>

class QProcess;
class QLocalSocket;
class PreviewResult;

class PreviewerBackend : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(PreviewerBackend)

    private:
        struct Task
        {
            bool repreview;
            QRectF rect;
            QString dir;

            bool operator==(const Task& t1)
            {
                return dir == t1.dir;
            }
        };

    public:
        static PreviewerBackend* instance();
        bool isWorking() const;
        using QObject::connect;

    public slots:
        void next();
        void start();
        void restart();
        void requestPreview(const QRectF& rect, const QString& dir);
        void requestRepreview(const QRectF& rect, const QString& dir);

    private slots:
        void connect();
        void onReadReady();
        void onBinaryMessageReceived(const QByteArray& data);

    signals:
        void stateChanged();
        void previewReady(const PreviewResult& result);

    private:
        void processMessage(const QString& type, QDataStream& in);

    private:
        PreviewerBackend();

    private:
        QProcess* _process;
        QLocalSocket* _socket;
        QList<Task> _taskList;
};

#endif // PREVIEWERBACKEND_H