#ifndef RUNMANAGER_H
#define RUNMANAGER_H

#include <QObject>
#include <QProcess>
#include <QBasicTimer>
#include <QDataStream>
#include <QWebSocket>
#include <QFutureWatcher>
#include <QTemporaryDir>

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
            if (!socket)
                return invalid;
            for (Device& device : devices) {
                if (device.socket == socket)
                    return device;
            }
            return invalid;
        }

        static Device& get(QList<Device>& devices, const QString& uid)
        {
            static Device invalid;
            if (uid.isEmpty())
                return invalid;
            for (Device& device : devices) {
                if (device.uid() == uid)
                    return device;
            }
            return invalid;
        }
    };

    struct UploadOperation
    {
        QTemporaryDir cacheDir;
        QFutureWatcher<size_t> zipWatcher;
        QFutureWatcher<void> uploadWatcher;
    };

public:
    enum DiscoveryCommands {
        Broadcast = 0x1100,
        Execute,
        Terminate,
        InfoReport,
        StartReport,
        OutputReport,
        FinishReport,
        ProgressReport,
        UploadStarted,
        ErrorReport
    };

public:
    static RunManager* instance();
    static QString recentDevice();
    static QVariantMap deviceInfo(const QString& uid);
    static bool isLocalDevice(const QString& uid);


public slots:
    static void terminate();
    static void execute(const QString& uid, const QString& projectDirectory);
    static void sendUploadStarted();
    static void sendProgressReport(int progress);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);

private:
    template <typename... Args>
    static void send(const QString& uid, DiscoveryCommands command, Args&&... args);
    static void cleanUploadOperationCache();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    explicit RunManager(QObject* parent = nullptr);
    ~RunManager() override;

signals:
    void deviceConnected(const QString& uid);
    void deviceDisconnected(const QString& uid);
    void deviceStarted();
    void deviceReadyOutput(const QString& output);
    void deviceFinished(int exitCode);
    void deviceErrorOccurred(const QString& errorString);
    void deviceUploadProgress(int progress);

    void processStarted();
    void processReadyOutput(const QString& output);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrorOccurred(QProcess::ProcessError error, const QString& errorString);

private:
    static RunManager* s_instance;
    static QBasicTimer s_broadcastTimer;
    static QUdpSocket* s_broadcastSocket;
    static QWebSocketServer* s_webSocketServer;
    static UploadOperation* s_uploadOperation;
    static QList<Device> s_devices;
    static QString s_recentDeviceUid;
};

Q_DECLARE_METATYPE(RunManager::DiscoveryCommands)

inline QDataStream& operator>>(QDataStream& in, RunManager::DiscoveryCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const RunManager::DiscoveryCommands& e)
{ return out << int(e); }

#endif // RUNMANAGER_H