#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QProcess>
#include <QBasicTimer>
#include <QDataStream>
#include <QWebSocket>
#include <utilityfunctions.h>

class QUdpSocket;
class QWebSocketServer;
class QWebSocket;

class DeviceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceManager)

    enum {
        BROADCAST_PORT = 15425,
        SERVER_PORT = 15426,
    };

    struct Device
    {
        QVariantMap info;
        QWebSocket* socket = nullptr;

        operator bool() const
        { return socket; }

        bool operator == (const Device& other)
        { return socket == other.socket; }

        QString uid() const
        { return info.value("deviceUid").toString(); }

        static Device get(const QList<Device>& devices, const QWebSocket* socket)
        {
            for (const Device& device : devices) {
                if (device.socket == socket)
                    return device;
            }
            return Device();
        }

        static Device get(const QList<Device>& devices, const QString& uid)
        {
            for (const Device& device : devices) {
                if (device.uid() == uid)
                    return device;
            }
            return Device();
        }
    };

    friend class ApplicationCore;

public:
    enum DiscoveryCommands {
        Broadcast = 0x1100,
        Execute,
        Terminate,
        InfoReport,
        StartReport,
        OutputReport,
        ExitReport
    };

public:
    static DeviceManager* instance();

    template<typename... Args>
    static void send(const QString& uid, DiscoveryCommands command, Args&&... args)
    {
        using namespace UtilityFunctions;
        if (const Device& device = Device::get(s_devices, uid)) {
            device.socket->sendBinaryMessage(push(command, push(std::forward<Args>(args)...)));
            return;
        }
        qWarning("WARNING: Cannot send any data, device is not connected, uid: %s", uid);
    }

private:
    explicit DeviceManager(QObject* parent = nullptr);
    ~DeviceManager() override;

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
    static QList<Device> s_devices;
};

Q_DECLARE_METATYPE(DeviceManager::DiscoveryCommands)

inline QDataStream& operator>>(QDataStream& in, DeviceManager::DiscoveryCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const DeviceManager::DiscoveryCommands& e)
{ return out << int(e); }

#endif // DEVICEMANAGER_H