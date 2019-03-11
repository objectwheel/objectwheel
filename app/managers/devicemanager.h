#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QProcess>
#include <QBasicTimer>
#include <QDataStream>

class QUdpSocket;
class QWebSocketServer;

class DeviceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceManager)

    enum {
        BROADCAST_PORT = 15425,
        SERVER_PORT = 15426,
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

Q_DECLARE_METATYPE(DeviceManager::DiscoveryCommands)

inline QDataStream& operator>>(QDataStream& in, DeviceManager::DiscoveryCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const DeviceManager::DiscoveryCommands& e)
{ return out << int(e); }

#endif // DEVICEMANAGER_H