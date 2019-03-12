#ifndef RUNMANAGER_H
#define RUNMANAGER_H

#include <QObject>
#include <QProcess>
#include <QBasicTimer>
#include <QDataStream>
#include <QWebSocket>
#include <utilityfunctions.h>

class QUdpSocket;
class QWebSocketServer;
class QWebSocket;

class RunManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RunManager)

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

        static Device& get(QList<Device>& devices, const QWebSocket* socket)
        {
            static Device invalid;
            for (Device& device : devices) {
                if (device.socket == socket)
                    return device;
            }
            return invalid;
        }

        static Device& get(QList<Device>& devices, const QString& uid)
        {
            static Device invalid;
            for (Device& device : devices) {
                if (device.uid() == uid)
                    return device;
            }
            return invalid;
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
        FinishReport
    };

public:
    static RunManager* instance();

public slots:
    void scheduleTermination(const QString& uid);
    void scheduleExecution(const QString& uid, const QString& projectDirectory);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);

private:
    template<typename... Args>
    static void send(const QString& uid, DiscoveryCommands command, Args&&... args)
    {
        using namespace UtilityFunctions;
        if (const Device& device = Device::get(s_devices, uid)) {
            device.socket->sendBinaryMessage(push(command, push(std::forward<Args>(args)...)));
            return;
        }
        qWarning("WARNING: Cannot send any data, device is not connected, uid: %s", uid.toUtf8().constData());
    }

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    explicit RunManager(QObject* parent = nullptr);
    ~RunManager() override;

signals:
    void deviceConnected(const QVariantMap& deviceInfo);
    void deviceDisconnected(const QString& uid);
    void projectStarted();
    void projectFinished(int exitCode);
    void projectReadyReadOutput(const QString& output);

private:
    static RunManager* s_instance;
    static QBasicTimer s_broadcastTimer;
    static QUdpSocket* s_broadcastSocket;
    static QWebSocketServer* s_webSocketServer;
    static QList<Device> s_devices;
};

Q_DECLARE_METATYPE(RunManager::DiscoveryCommands)

inline QDataStream& operator>>(QDataStream& in, RunManager::DiscoveryCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const RunManager::DiscoveryCommands& e)
{ return out << int(e); }

#endif // RUNMANAGER_H