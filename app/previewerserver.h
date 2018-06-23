#ifndef PREVIEWERSOCKET_H
#define PREVIEWERSOCKET_H

#include <QLocalServer>

class QTimer;
class QDataStream;

class PreviewerServer final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PreviewerServer)

    using QObject::connect;

public:
    explicit PreviewerServer(QObject* parent = nullptr);

    void listen(const QString& serverName);
    void send(int code, const QByteArray& data = QByteArray());

signals:
    void connectionTimeout();
    void disconnected();
    void dataArrived(int code, QDataStream& dataStream);

private slots:
    void onNewConnection();
    void onReadReady();

private:
    int m_blockSize;
    QLocalServer* m_server;
    QTimer* m_checkAliveTimer;
    QPointer<QLocalSocket> socket;
};

#endif // PREVIEWERSOCKET_H