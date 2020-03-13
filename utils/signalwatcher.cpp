#include <signalwatcher.h>
#include <csignal> // std::signal
#include <cstdio>  // std::fprintf, stderr
#include <cstdlib> // std::_Exit, EXIT_FAILURE
#include <cstring> // std::strlen
#include <utility> // std::forward

#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QRandomGenerator>

#if defined(Q_OS_WINDOWS)
#  define STDERR_FILENO 2
#  include <io.h>     // ::_write
#elif defined(Q_OS_UNIX)
#  include <unistd.h> // ::write, STDERR_FILENO
#else
#  error Cannot find an equivalent function to POSIX write() [which is async-signal-safe]
#endif

template <typename... Args>
static void qWrite(Args&&... args)
{
#if defined(Q_OS_WINDOWS)
    (void) ::_write(std::forward<Args>(args)...);
#else
    (void) ::write(std::forward<Args>(args)...);
#endif
}

// Converts base(10) int to char[], writes into str and returns the address of str
static char* itoa(int num, char* str)
{
    int copy = num;
    int pos = 0;
    int digits = 0;
    do {
        ++digits;
        copy /= 10;
    } while (copy != 0);
    if (num < 0) {
        digits += 2;
        str[pos++] = '-';
    }
    do {
        char rem = num % 10;
        str[digits - pos - 1] = '0' + (rem < 0 ? -rem : rem);
        num /= 10;
        ++pos;
    } while (num != 0);
    str[pos] = '\0';
    return str;
}

// Inserts src into dst at the position where sym is found (beware internal buffer size)
static char* change(char* dst, char sym, const char* src)
{
    if (sym == '\0')
        return dst;
    int spos = 0;
    for (; dst[spos] != sym && dst[spos] != '\0'; ++spos);
    if (dst[spos] == '\0')
        return dst;
    int pos = spos + 1;
    for (; dst[pos] != '\0'; ++pos);
    int size = pos - spos;
    char buff[100]; // No variable length arrays in C++
    if (size > (int) sizeof(buff))
        return dst;
    for (int i = 0; i < size; ++i)
        buff[i] = dst[spos + 1 + i];
    pos = 0;
    for (; src[pos] != '\0'; ++pos);
    for (int i = 0; i < pos; ++i)
        dst[spos++] = src[i];
    for (int i = 0; i < (int) sizeof(buff); ++i)
        dst[spos++] = buff[i];
    return dst;
}

std::atomic<qintptr>
SignalWatcher::s_socketDescriptor;
SignalWatcher::SignalWatcher(QObject* parent) : QObject(parent)
{
    // Everything we reference from signal handler must be lock-free atomic
    // If not, it may still work, but may also yield undefined behavior.
    Q_ASSERT(s_socketDescriptor.is_lock_free());

    /** Prepare Socket Pair **/
    // We are using a pair of QLocalServer/QLocalSocket for interrupting
    // the main event loop and notifying against the ongoing signals.
    auto server = new QLocalServer(this);
    auto socket = new QLocalSocket(this);
    server->listen(QString::number(QRandomGenerator::global()->generate()));
    socket->connectToServer(server->serverName(), QIODevice::ReadOnly);
    server->waitForNewConnection();
    socket->waitForConnected();
    Q_ASSERT(server->hasPendingConnections());
    s_socketDescriptor = server->nextPendingConnection()->socketDescriptor();
    connect(socket, &QLocalSocket::readyRead, this, &SignalWatcher::onSocketReadyRead);

    /** Cover ISO Standard C/C++ Signals **/
    // According to the standard, signal handlers for following signals
    // (which are computational exceptions) cannot "return" (hence cannot
    // be ignored), they must exit the application at the end. It only
    // gives you a few basic final operations like writing error messages
    // before exiting the app. (only async-signal-safe exit functions
    // can be called, but ie. quickexit() is not defined by macOS's libc
    // at the moment. So we call _Exit function in the handleFailure function.
    std::signal(SIGFPE, handleFailure);
    std::signal(SIGILL, handleFailure);
    std::signal(SIGSEGV, handleFailure);

    // According to standard, following signals can be ignored safely
    // and main program could safely continue its execution from where
    // it is left last time. So, in handleInterrupt function we just
    // write a few bytes to the local server and then return from the
    // handler (so ignoring the signal). This way, execution can continue
    // in the main program and the socket will get the incoming bytes,
    // so main program's event loop will be notified about the signal.
    std::signal(SIGABRT, handleInterrupt);
    std::signal(SIGINT, handleInterrupt);
    std::signal(SIGTERM, handleInterrupt);
}

SignalWatcher* SignalWatcher::instance()
{
    static SignalWatcher instance;
    return &instance;
}

void SignalWatcher::defaultInterruptAction(int signal) const
{
    // Default interrupt action prints a message then exits the main
    // event loop and hence application and the stack unwinds properly.
    std::fprintf(stderr, "\nApplication terminated with signal(%d)", signal);
    std::fprintf(stderr, "\nGoodbye\n");
    QCoreApplication::exit(EXIT_FAILURE);
}

void SignalWatcher::onSocketReadyRead() const
{
    if (auto socket = qobject_cast<QLocalSocket*>(sender())) {
        int sig;
        socket->read((char*) &sig, (int) sizeof(sig));
        emit signal(sig);
    }
}

void SignalWatcher::handleFailure(int signal) noexcept
{
    // According to the standard, you cannot recover from SIGFPE, SIGILL
    // and SIGSEGV (computational exceptions); at least in a portable way.
    // So we are just printing an error message and then exiting the app.
    char sigbuff[15];
    char message[50] = "\nApplication failed with signal(@)";
    char goodbye[  ] = "\nGoodbye\n";
    change(message, '@', itoa(signal, sigbuff));
    qWrite(STDERR_FILENO, message, (unsigned int) std::strlen(message));
    qWrite(STDERR_FILENO, goodbye, (unsigned int) std::strlen(goodbye));
    std::_Exit(EXIT_FAILURE);
}

void SignalWatcher::handleInterrupt(int signal) noexcept
{
    // Write signal value to the server and leave the function
    // without exiting the app. So main program's event loop will
    // continue and process the interrupt signal in a proper way.
    qWrite(s_socketDescriptor, &signal, (unsigned int) sizeof(signal));
}
