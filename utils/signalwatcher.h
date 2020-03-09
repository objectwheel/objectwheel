#ifndef SIGNALWATCHER_H
#define SIGNALWATCHER_H

#include <atomic>
#include <QObject>

class SignalWatcher final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SignalWatcher)

public:
    static SignalWatcher* instance();

public slots:
    void defaultInterruptAction(int signal) const;

private slots:
    void onSocketReadyRead() const;

private:
    static void handleFailure(int signal) noexcept;
    static void handleInterrupt(int signal) noexcept;

signals:
    void signal(int signal) const;

private:
    explicit SignalWatcher(QObject* parent = nullptr);

private:
    static std::atomic<qintptr> s_socketDescriptor;
};

#endif // SIGNALWATCHER_H
