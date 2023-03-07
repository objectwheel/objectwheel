#include "signalhandler.h"

#include <csignal>     // std::signal
#include <cstdio>      // std::fprintf, stderr
#include <cstdlib>     // std::_Exit, EXIT_FAILURE
#include <cstring>     // std::memmove, std::memcpy, std::strlen, std::strchr
#include <limits>      // std::numeric_limits
#include <type_traits> // std::enable_if

#include <qplatformdefs.h> // QT_WRITE
#include <QLocalServer>
#include <QLocalSocket>
#include <QRandomGenerator>
#include <QCoreApplication>

#if defined(Q_OS_WINDOWS)
#   include <windows.h>
#   undef QT_WRITE
#   define QT_WRITE(fd, data, len) WriteFile(HANDLE((fd)), (data), DWORD(len), 0, NULL)
#   define STDERR_FILENO 2
#endif

namespace {

void replace(char* dest, char ch, const char* src) noexcept
{
    if (dest == nullptr || ch == 0 || src == nullptr)
        return;

    auto len = std::strlen(src);
    while ((dest = std::strchr(dest, ch))) {
        std::memmove(dest + len, dest + 1, std::strlen(dest + 1) + 1);
        std::memcpy(dest, src, len);
        dest += len;
    }
}

template <typename T, std::size_t N, typename = std::enable_if_t<std::is_integral_v<T>>>
char* int_to_chars(char (&d)[N], T v) noexcept
{
    static_assert(N > 1, "N must be greater than 1");
    auto i = N;
    d[--i] = 0;
    if (v < 0) {
        do {
            d[--i] = '0' - v % 10;
            v /= 10;
        } while (v && i);
        if (i > 0)
            d[--i] = '-';
    } else {
        do {
            d[--i] = '0' + v % 10;
            v /= 10;
        } while (v && i);
    }
    return &d[i];
}
}

quintptr SignalHandler::s_socketDescriptor = 0;

SignalHandler::SignalHandler(QObject* parent) : QObject(parent)
{
    Q_ASSERT_X(!s_socketDescriptor, "SignalHandler", "Multiple instances are not allowed");

    /** Attact to failure signals fist **/
    // According to the standard, the following signals (which are computational
    // exceptions) cannot be ignored. They must exit the application at the end.
    // You can only perform a few basic final operations, such as writing error
    // messages, before exiting the app. Only async-signal-safe exit functions
    // can be called, but quickexit() is not defined by macOS's libc at the moment.
    // Therefore, we call the _Exit function in the handleFailure function.
    std::signal(SIGFPE, handleFailure);
    std::signal(SIGILL, handleFailure);
    std::signal(SIGSEGV, handleFailure);

    /** Prepare local connection **/
    // We are using a pair of QLocalServer/QLocalSocket for interrupting
    // the main event loop and notifying against the ongoing signals.
    auto server = new QLocalServer(this);
    auto socket = new QLocalSocket(this);
    // Server name needs to be a full path to a temporary location on iOS
    // Simulator. Because the Qt implementation prepends the server name
    // with a full path to a temporary location internally, and on Simulator
    // the usual temporary location is too long to store in the sockaddr_un's
    // sunpath[104] buffer (which is used by Qt on UNIX to establish a socket)
    server->listen(
        #if TARGET_OS_SIMULATOR
            u"/tmp/"_s +
        #endif
        QString::number(QRandomGenerator::global()->generate())
    );
    Q_ASSERT(server->isListening());
    socket->connectToServer(server->serverName(), QIODevice::WriteOnly);
    server->waitForNewConnection();
    Q_ASSERT(server->hasPendingConnections());
    connect(server->nextPendingConnection(), &QLocalSocket::readyRead,
            this, &SignalHandler::onSocketReadyRead);
    s_socketDescriptor = socket->socketDescriptor();

    /** Attact rest of the standard C++ signals **/
    // According to standard, following signals can be ignored safely
    // and main program could safely continue its execution from where
    // it is left off recently. So, in handleInterrupt function we just
    // write a few bytes to the local server and then return from the
    // handler (so ignoring the signal). This way, execution can continue
    // in the main program and the socket will get the incoming bytes,
    // so main program's event loop will be notified about the signal.
    std::signal(SIGABRT, handleInterrupt);
    std::signal(SIGINT, handleInterrupt);
    std::signal(SIGTERM, handleInterrupt);
}

void SignalHandler::exitGracefully(int signal) const
{
    // Default interrupt action prints a message then terminates the main
    // event loop and hence the application and stack can unwind properly.
    std::fprintf(stderr, "Application quit with signal(%d)\nGoodbye\n", signal);
    QCoreApplication::exit(EXIT_SUCCESS);
}

void SignalHandler::onSocketReadyRead() const
{
    if (auto socket = qobject_cast<QLocalSocket*>(sender())) {
        int sig;
        socket->read((char*) &sig, sizeof(sig));
        emit interrupted(sig);
    }
}

void SignalHandler::handleFailure(int signal) noexcept
{
    // According to the standard, it is not possible to handle certain computational
    // exceptions (SIGFPE, SIGILL, and SIGSEGV) in a portable way. Therefore, the
    // application will simply print an error message and exit. For more information,
    // see https://en.cppreference.com/w/cpp/utility/program/signal.
    static constexpr char message[] = "Application failed with signal(@)\nGoodbye\n";
    char sigbuff[std::numeric_limits<int>::digits10 + 3]; // +1 max digits +1 null +1 sign
    char msgbuff[sizeof(message) + sizeof(sigbuff)];
    std::memcpy(msgbuff, message, sizeof(message));
    replace(msgbuff, '@', int_to_chars(sigbuff, signal));
    std::ignore = QT_WRITE(STDERR_FILENO, msgbuff, std::strlen(msgbuff));
    std::_Exit(EXIT_FAILURE);
}

void SignalHandler::handleInterrupt(int signal) noexcept
{
    // Write the signal value to the socket and leave the function
    // without exiting the app. So main program's event loop will
    // continue and process the interrupt signal in a proper way.
    std::ignore = QT_WRITE(s_socketDescriptor, &signal, sizeof(signal));
}
