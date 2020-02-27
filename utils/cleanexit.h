#ifndef CLEANEXIT_H
#define CLEANEXIT_H

#include <QCoreApplication>
#include <csignal>

struct CleanExit
{
    CleanExit()
    {
        std::signal(SIGTERM, exit);
        std::signal(SIGSEGV, exit);
        std::signal(SIGINT, exit);
        std::signal(SIGILL, exit);
        std::signal(SIGABRT, exit);
        std::signal(SIGFPE, exit);
    }

    ~CleanExit()
    {
        qWarning("Exited properly");
    }

    static void exit(int)
    {
        static bool isExited = false;
        if (!isExited) {
            isExited = true;
            QCoreApplication::exit(EXIT_FAILURE);
        }
    }
};

#endif // CLEANEXIT_H
