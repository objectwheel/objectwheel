#ifndef PREVIEWERBACKEND_H
#define PREVIEWERBACKEND_H

#include <QObject>

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
        PreviewerBackend();

    private:
        QProcess* _process;
        QLocalSocket* _socket;
        bool _isWorking;
};

#endif // PREVIEWERBACKEND_H