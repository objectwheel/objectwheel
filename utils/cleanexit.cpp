#include <cleanexit.h>
#include <csignal>
#include <QCoreApplication>

#if defined(Q_OS_WINDOWS)
# include <io.h>
#elif defined(Q_OS_UNIX)
# include <unistd.h>
#endif

std::jmp_buf CleanExit::s_jumpBuffer;
SignalHandler CleanExit::s_signalHandler = nullptr;

CleanExit::CleanExit()
{
    // Register ISO Standard C++ Signals
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE, signalHandler);
    std::signal(SIGILL, signalHandler);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

int CleanExit::exec(QCoreApplication& app, SignalHandler signalHandler)
{
    s_signalHandler = signalHandler;
    static CleanExit cleanExit;
    Q_UNUSED(cleanExit)
    if(setjmp(s_jumpBuffer) > 0)
        return EXIT_FAILURE;
    else
        return app.exec();
}

void CleanExit::signalHandler(int signal)
{
    bool exit = true;
    if (s_signalHandler)
        exit = s_signalHandler(signal);
    if (exit)
        std::longjmp(s_jumpBuffer, 1);
}

bool CleanExit::defaultSignalHandler(int signal)
{
    QString goodbye("Quit the application by signal(%1)\nGoodbye\n");
    goodbye = goodbye.arg(QString::number(signal));
    ::write(STDERR_FILENO, goodbye.toLatin1().constData(), goodbye.size());
    return true;
}
