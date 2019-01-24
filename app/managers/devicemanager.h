#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QBasicTimer>

class QUdpSocket;
class QWebSocketServer;

class DeviceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceManager)

    friend class ApplicationCore;

    enum {
        BROADCAST_PORT = 15425,
        SERVER_PORT = 15426,
    };
    static const char* const UID_PROPERTY;
    static const QByteArray SERVER_NAME;
    static const QByteArray BROADCAST_MESSAGE;

private:
    explicit DeviceManager(QObject* parent = nullptr);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);

protected:
    void timerEvent(QTimerEvent* event) override;

signals:
    void connected(const QVariantMap& deviceInfo);
    void disconnected(const QString& uid);

private:
    static QBasicTimer s_broadcastTimer;
    static QUdpSocket* s_broadcastSocket;
    static QWebSocketServer* s_webSocketServer;
};

#endif // DEVICEMANAGER_H