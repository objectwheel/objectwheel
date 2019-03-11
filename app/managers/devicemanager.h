#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QProcess>
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
    static const QByteArray UID_PROPERTY;
    static const QByteArray SERVER_NAME;

public:
    static DeviceManager* instance();

    static const QList<QVariantMap>& deviceInfoList();
    static QVariantMap deviceInfo(const QString& uid);
    static bool deviceInfoExists(const QString& uid);

private:
    explicit DeviceManager(QObject* parent = nullptr);
    ~DeviceManager() override;

private:
    static void removeDeviceInfo(const QString& uid);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);

protected:
    void timerEvent(QTimerEvent* event) override;

signals:
    void connected(const QVariantMap& deviceInfo);
    void disconnected(const QString& uid);

    void readyReadStandardError();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void started();

private:
    static DeviceManager* s_instance;
    static QBasicTimer s_broadcastTimer;
    static QUdpSocket* s_broadcastSocket;
    static QWebSocketServer* s_webSocketServer;
    static QList<QVariantMap> s_deviceInfoList;
};

#endif // DEVICEMANAGER_H