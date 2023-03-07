#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <utils_global.h>
#include <QObject>

class UTILS_EXPORT SignalHandler final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SignalHandler)

public:
    explicit SignalHandler(QObject* parent = nullptr);

public slots:
    void exitGracefully(int signal) const;

private slots:
    void onSocketReadyRead() const;

private:
    static void handleFailure(int signal) noexcept;
    static void handleInterrupt(int signal) noexcept;

signals:
    void interrupted(int signal) const;

private:
    static quintptr s_socketDescriptor;
};

#endif // SIGNALHANDLER_H
