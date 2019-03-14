#ifndef RUNMANAGER_H
#define RUNMANAGER_H

#include <QObject>
#include <QProcess>
#include <QBasicTimer>
#include <QDataStream>
#include <QWebSocket>

class QUdpSocket;
class QWebSocketServer;
class QWebSocket;

class RunManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RunManager)

    friend class ApplicationCore;

    enum {
        BROADCAST_PORT = 15425,
        SERVER_PORT = 15426,
    };

    struct Device
    {
        QVariantMap info;
        QProcess* process = nullptr;
        QWebSocket* socket = nullptr;

        operator bool() const
        { return !uid().isEmpty(); }

        bool operator == (const Device& other)
        { return uid() == other.uid(); }

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
    static QVariantMap deviceInfo(const QString& uid);
    static bool isLocalDevice(const QString& uid);

public slots:
    static void terminate();
    static void execute(const QString& uid, const QString& projectDirectory);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);

private:
    template <typename... Args>
    static void send(const QString& uid, DiscoveryCommands command, Args&&... args);

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    explicit RunManager(QObject* parent = nullptr);
    ~RunManager() override;

signals:
    void deviceConnected(const QString& uid);
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
    static QString s_recentDeviceUid;
};

Q_DECLARE_METATYPE(RunManager::DiscoveryCommands)

inline QDataStream& operator>>(QDataStream& in, RunManager::DiscoveryCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const RunManager::DiscoveryCommands& e)
{ return out << int(e); }

#endif // RUNMANAGER_H