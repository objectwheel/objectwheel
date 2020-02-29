#include <signalwatcher.h>
#include <csignal>

#include <QLocalSocket>
#include <QLocalServer>
#include <QRandomGenerator>

#if defined(Q_OS_WINDOWS)
#  include <io.h>
#elif defined(Q_OS_UNIX)
#  include <unistd.h>
#else
#  error Cannot find an equivalent function to POSIX's write() [which is async-signal-safe]
#endif

template <typename... Args> static auto qWrite(Args&&... args)
{
#if defined(Q_OS_WINDOWS)
    return ::_write(std::forward<Args>(args)...);
#else
    return ::write(std::forward<Args>(args)...);
#endif
}

QLocalServer* SignalWatcher::s_server = nullptr;
QLocalSocket* SignalWatcher::s_socket = nullptr;
std::atomic<qintptr> SignalWatcher::s_socketDescriptor;

SignalWatcher::SignalWatcher(QObject* parent) : QObject(parent)
{
    // Everything we reference from signal handler must be lock-free atomic
    Q_ASSERT(s_socketDescriptor.is_lock_free());

    // Prepare sockets (we write to the server and we listen to the socket)
    s_server = new QLocalServer(this);
    s_socket = new QLocalSocket(this);
    s_server->listen(QString::number(QRandomGenerator::global()->generate()));
    s_socket->connectToServer(s_server->serverName(), QIODevice::ReadOnly);
    s_server->waitForNewConnection();
    s_socket->waitForConnected();
    Q_ASSERT(s_server->hasPendingConnections());
    s_socketDescriptor = s_server->nextPendingConnection()->socketDescriptor();
    connect(s_socket, &QLocalSocket::readyRead, this, &SignalWatcher::onSocketReadyRead);

    // ISO Standard C/C++ Signals
    std::signal(SIGABRT, handleSignal);
    std::signal(SIGFPE, handleSignal);
    std::signal(SIGILL, handleSignal);
    std::signal(SIGINT, handleSignal);
    std::signal(SIGSEGV, handleSignal);
    std::signal(SIGTERM, handleSignal);
}

SignalWatcher* SignalWatcher::instance()
{
    static SignalWatcher signalWatcher;
    return &signalWatcher;
}

void SignalWatcher::onSocketReadyRead()
{
    int sig;
    s_socket->read((char*)&sig, sizeof(sig));
    emit signal(sig);
}

void SignalWatcher::handleSignal(int signal)
{
    qWrite(s_socketDescriptor, &signal, sizeof(signal));
}
