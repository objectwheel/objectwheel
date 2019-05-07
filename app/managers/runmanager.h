#ifndef RUNMANAGER_H
#define RUNMANAGER_H

#include <QProcess>
#include <QBasicTimer>
#include <QDataStream>
#include <QFutureWatcher>
#include <QTemporaryDir>

class QUdpSocket;
class QWebSocket;
class QWebSocketServer;

class RunManager final : public QObject
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

    struct UploadInfo
    {
        bool canceled;
        QString fileName;
        QString deviceUid;
        QString projectDir;
        QFutureWatcher<size_t> watcher;
        QScopedPointer<QTemporaryDir> cacheDir;
    };

public:
    enum DiscoveryCommands : quint32 {
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
    static QString recentProjectDirectory();
    static QVariantMap deviceInfo(const QString& uid);
    static bool isLocalDevice(const QString& uid);

    static void sendTerminate();
    static void sendUploadStarted();
    static void sendProgressReport(int progress);
    static void sendExecute(const QString& uid, const QString& projectDirectory);
    static void scheduleUploadCancelation();


private slots:
    void onNewConnection();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);

private:
    template <typename... Args>
    static void send(const QString& uid, DiscoveryCommands command, Args&&... args);
    static void upload();
    void timerEvent(QTimerEvent* event) override;

private:
    explicit RunManager(QObject* parent = nullptr);
    ~RunManager() override;

signals:
    void deviceConnected(const QString& uid);
    void deviceDisconnected(const QVariantMap& deviceInfo);

    void applicationStarted();
    void applicationUploadProgress(int progress);
    void applicationReadyOutput(const QString& output);
    void applicationErrorOccurred(QProcess::ProcessError error, const QString& errorString);
    void applicationFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    static RunManager* s_instance;
    static QBasicTimer s_broadcastTimer;
    static QBasicTimer s_remoteExecutionTimer;
    static QUdpSocket* s_broadcastSocket;
    static QWebSocketServer* s_webSocketServer;
    static UploadInfo s_uploadInfo;
    static QList<Device> s_devices;
    static QString s_recentDeviceUid;
    static QString s_recentProjectDirectory;
};

#endif // RUNMANAGER_H