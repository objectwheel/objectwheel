#ifndef PREVIEWERBACKEND_H
#define PREVIEWERBACKEND_H

#include <QObject>
#include <QDataStream>

class QProcess;
class QLocalSocket;
class PreviewResult;

class PreviewerBackend : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(PreviewerBackend)

    public:
        static PreviewerBackend* instance();
        bool isWorking() const;
        using QObject::connect;

    public slots:
        void start();
        void restart();
        void requestPreview(const QRectF& rect, const QString& dir);

    private slots:
        void connect();
        void onReadReady();

    signals:
        void stateChanged();
        void previewReady(const PreviewResult& result);

    private:
        void processMessage(const QString& type);

    private:
        PreviewerBackend();

    private:
        QDataStream stream;
        QProcess* _process;
        QLocalSocket* _socket;
        bool _isWorking;
};

#endif // PREVIEWERBACKEND_H